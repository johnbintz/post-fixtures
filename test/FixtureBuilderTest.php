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
					array('categories', array('test,test2/test3')),
					array('metadata', array('key', array('metadata' => 'value'))),
					array('post', array('Post title 2')),
					array('date', array('2010-01-02')),
				),
				array(
					'post' => array(
						 array(
							'post_title' => 'Post title',
							'post_type'  => 'post',
						 	'post_date'  => '2010-01-01',
							'categories' => array('test', 'test2/test3'),
						 	'metadata' => array('key' => array('metadata' => 'value'))
						),
						 array(
							'post_title' => 'Post title 2',
							'post_type'  => 'post',
						 	'post_date'  => '2010-01-02',
						),
					),
				),
				array(true, true, true, true, true, true)
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
						'content',
						array('Post content')
					),
					array(
						'categories',
						array('test')
					),
					array(
						'metadata',
						array('test', 'test2')
					),
					array(
						'tags',
						array('tag1,tag2')
					)
				),
				(object)array(
					'post_title' => 'Post title',
					'post_content' => 'Post content',
					'post_type' => 'post',
					'post_date' => '2010-01-01',
					'post_status' => 'publish',
					'ID' => 1
				),
				array(
					'posts' => array(
						1 => (object)array(
							'post_title' => 'Post title',
							'post_content' => 'Post content',
							'post_type' => 'post',
							'post_date' => '2010-01-01',
							'post_status' => 'publish',
							'ID' => 1
						)
					),
					'categories' => array(
						1 => 'test'
					),
					'tags' => array(
						1 => array('tag1', 'tag2')
					),
					'post_meta' => array(
						1 => array(
							'test' => 'test2'
						)
					)
				)
			),
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

		$this->assertTrue(!isset($builder->current_object));
	}

	function testBuildEmpty() {
		$builder = new FixtureBuilder();

		$builder->build();
	}

	function testFinalize() {
		$fb = $this->getMock('FixtureBuilder', array('defer', 'build_category', 'build_post'));

		$fb->deferred_builds = array(
			'category' => array('test', 'test2'),
			'post' => array('post3', 'post4')
		);

		$fb->expects($this->at(0))->method('defer');
		$fb->expects($this->at(1))->method('build_category')->with(array('name' => 'test'));
		$fb->expects($this->at(2))->method('build_category')->with(array('name' => 'test2'));
		$fb->expects($this->at(3))->method('build_post')->with(array('post' => 'post3'));
		$fb->expects($this->at(4))->method('build_post')->with(array('post' => 'post4'));

		$fb->finalize();
	}

	function providerTestOption() {
		return array(
			array('test', 'value', 'value'),
			array('test2', false, false)
		);
	}

	/**
	 * @dataProvider providerTestOption
	 */
	function testOption($key, $value, $expected_value) {
		$fb = new FixtureBuilder();
		$fb->option($key, $value);

		$this->assertEquals($expected_value, get_option($key));
	}
}
