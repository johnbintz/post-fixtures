<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../classes/FixtureBuilder.inc');

class FixtureBuilderTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

	function providerTestDeferBuild() {
		return array(
			array(
				array(
					array('category', array('test'))
				),
				array('category' => array('test')),
				array(true)
			),
			array(
				array(
					array('category', array('test,test2'))
				),
				array('category' => array('test', 'test2')),
				array(true)
			),
			array(
				array(
					array('category', array('test')),
					array('post', array('Post title')),
				),
				array('category' => array('test')),
				array(true, false)
			),
			array(
				array(
					array('category', array('test')),
					array('category', array('test2')),
				),
				array('category' => array('test', 'test2')),
				array(true, true)
			),
			array(
				array(
					array('post', array('Post title')),
					array('date', array('2010-01-01')),
					array('categories', array('test,test2')),
					array('metadata', array('key', array('metadata' => 'value'))),
				),
				array(
					'post' => array(
						 array(
							'post_title' => 'Post title',
							'post_type'  => 'post',
						 	'post_date'  => '2010-01-01',
							'categories' => array('test', 'test2'),
						 	'metadata' => array('key' => array('metadata' => 'value'))
						)
					),
				),
				array(true, true, true, true)
			),
		);
	}

	/**
	 * @dataProvider providerTestDeferBuild
	 */
	function testDeferBuild($instructions, $expected_deferred_build, $expected_exceptions) {
		$builder = new FixtureBuilder();

		foreach ($instructions as $info) {
			list($instruction, $parameters) = $info;
			$expected_exception = array_shift($expected_exceptions);

			try {
				$return = call_user_func_array(array($builder, $instruction), $parameters);
				$this->assertTrue($expected_exception);
			} catch (Exception $e) {
				$this->assertFalse($expected_exception, $e->getMessage());
				$this->assertType('LogicException', $e);
			}
		}

		$builder->defer();

		$this->assertEquals($expected_deferred_build, $builder->deferred_builds);
	}

	function providerTestImmediateBuild() {
		return array(
			array(
				array(
					array(
						'category',
						array('test'),
					),
					array(
						'category',
						array('test/test2'),
					)
				),
				array('test' => 1, 'test/test2' => 2),
				array(
					'categories' => array(
						1 => 'test',
						2 => 'test2'
					)
				)
			),
			array(
				array(
					array(
						'post',
						array('Post title')
					),
					array(
						'date',
						array('2010-01-01')
					),
					array(
						'categories',
						array('test')
					),
					array(
						'metadata',
						array('test', 'test2')
					)
				),
				(object)array(
					'post_title' => 'Post title',
					'post_type' => 'post',
					'post_date' => '2010-01-01',
					'post_status' => 'publish',
				'ID' => 1
				),
				array(
					'posts' => array(
						1 => (object)array(
							'post_title' => 'Post title',
							'post_type' => 'post',
							'post_date' => '2010-01-01',
							'post_status' => 'publish',
							'ID' => 1
						)
					),
					'categories' => array(
						1 => 'test'
					),
					'post_meta' => array(
						1 => array(
							'test' => 'test2'
						)
					)
				)
			)
		);
	}

	/**
	 * @dataProvider providerTestImmediateBuild
	 */
	function testImmediateBuild($instructions, $expected_return, $wp_checks) {
		$builder = new FixtureBuilder();
		foreach ($instructions as $info) {
			list($instruction, $parameters) = $info;
			call_user_func_array(array($builder, $instruction), $parameters);
		}
		$this->assertEquals($expected_return, $builder->build());

		foreach ($wp_checks as $type => $info) {
			switch ($type) {
				case 'posts':
					foreach ($info as $key => $post) {
						$this->assertEquals($post, get_post($key));
					}
					break;
				case 'categories':
					foreach ($info as $key => $category_name) {
						$this->assertEquals($category_name, get_cat_name($key));
					}
					break;
				case 'post_meta':
					foreach ($info as $post_id => $meta_info) {
						foreach ($meta_info as $key => $value) {
							$this->assertEquals($value, get_post_meta($post_id, $key, true));
						}
					}
					break;
			}
		}
	}
}
