<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../classes/PostFixtures.inc');

class PostFixturesTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();

		$this->pf = new PostFixtures();
	}

	function providerTestParseJSON() {
		return array(
			array(false, false),
			array('', false),
			array('{]', false),
			array('{}', array()),
			array('[]', array())
		);
	}

	/**
	 * @dataProvider providerTestParseJSON
	 */
	function testParseJSON($input, $expected_output) {
		$this->assertEquals($expected_output, $this->pf->parse_json($input));
	}
}
