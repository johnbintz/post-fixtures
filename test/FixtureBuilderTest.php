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
					array('category', array('test/test2'))
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
					array('date', array('2010-01-01'))
				),
				array('post' => array(
					array(
						'post_title' => 'Post title',
						'post_date'  => '2010-01-01',
						'post_type'  => 'post',
					)
				)),
				array(true, true)
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
				$this->assertFalse($expected_exception);
				$this->assertType('LogicException', $e);
			}
		}

		$builder->defer();

		$this->assertEquals($expected_deferred_build, $builder->deferred_builds);
	}
}
