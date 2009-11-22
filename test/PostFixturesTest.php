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
			array('[]', array()),
			array('["test", "test2"]', array('test', 'test2')),
			array('{"one": "test", "two": "test2"}', array('test', 'test2'))
		);
	}

	/**
	 * @dataProvider providerTestParseJSON
	 */
	function testParseJSON($input, $expected_output) {
		$this->assertEquals($expected_output, $this->pf->parse_json($input));
	}

	function testRemoveAllPosts() {
		global $wp_test_expectations;

		$posts = array();

		for ($i = 0; $i < 5; ++$i) {
			$post = (object)array('ID' => $i);
			wp_insert_post($post);
			$posts[] = $post;
			update_post_meta($i, md5(rand()), md5(rand()));
		}

		_set_up_get_posts_response('nopaging=1', $posts);

		$this->assertEquals(5, count($wp_test_expectations['posts']));
		$this->assertEquals(5, count($wp_test_expectations['post_meta']));

		$this->pf->remove_all_posts();

		$this->assertEquals(0, count($wp_test_expectations['posts']));
		$this->assertEquals(0, count($wp_test_expectations['post_meta']));
	}

	function testRemoveAllCategories() {
		global $wp_test_expectations;
		update_option('default_category', 0);

		for ($i = 0; $i < 5; ++$i) {
			add_category($i, (object)array('slug' => 'test-' . $i));
		}

		$this->assertEquals(5, count($wp_test_expectations['categories']));

		$this->pf->remove_all_categories();

		$this->assertEquals(1, count($wp_test_expectations['categories']));
		$result = get_category(0);
		$this->assertTrue(isset($result->term_id));
	}
}
