<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('vfsStream/vfsStream.php');
require_once(dirname(__FILE__) . '/../classes/PostFixtures.inc');

class PostFixturesTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();

		$this->pf = new PostFixtures();
		$_POST = array();

		vfsStreamWrapper::register();
		vfsStreamWrapper::setRoot(new vfsStreamDirectory('root'));
	}

	function tearDown() {
		$_POST = array();
	}

	function providerTestParseJSON() {
		return array(
			array(false, false),
			array('', false),
			array('{]', false),
			array('{}', array()),
			array('[]', array()),
			array('["test", "test2"]', array('test', 'test2')),
			array('{"one": "test", "two": "test2"}', array('one' => 'test', 'two' => 'test2'))
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

		_set_up_get_posts_response(array('numberposts' => '-1', 'post_status' => 'draft,pending,future,inherit,private,publish'), $posts);

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

	function providerTestProcessData() {
		return array(
			array(
				array(
					'posts' => array(
						array(
							'title' => 'test',
							'categories' => array('test1', 'test2'),
							'date' => '2009-01-01',
							'metadata' => array(
								'test' => 'test2'
							)
						)
					)
				),
				array(
					'posts' => array(
						array(
							'title' => 'test',
							'categories' => array('test1', 'test2'),
							'date' => '2009-01-01',
							'metadata' => array(
								'test' => 'test2'
							)
						)
					),
					'categories' => array('test1', 'test2'),
					'options' => array()
				)
			),
			array(
				array(
					'options' => array(
						'test' => 'test2'
					)
				),
				array(
					'posts' => array(),
					'categories' => array(),
					'options' => array(
						'test' => 'test2'
					)
				)
			)
		);
	}

	/**
	 * @dataProvider providerTestProcessData
	 */
	function testProcessData($data, $expected_output) {
		$this->assertEquals($expected_output, $this->pf->process_data($data));
	}

	function providerTestCreateCategories() {
		return array(
			array(false, array()),
			array(array(), array()),
			array(array('test'), array('test' => 1)),
			array(array('test/test2'), array('test' => 1, 'test/test2' => 2)),
		);
	}

	/**
	 * @dataProvider providerTestCreateCategories
	 */
	function testCreateCategories($input, $expected_output) {
		$this->assertEquals($expected_output, $this->pf->create_categories($input));
	}

	function providerTestCreatePosts() {
		return array(
			array(false, array(), false),
			array(
				array(),
				array()
			),
			array(
				array(
					array(
						'post_title' => 'test'
					)
				),
				array(1),
				array(1 => array(1))
			),
			array(
				array(
					array(
						'post_title' => 'test',
						'categories' => array('test2')
					)
				),
				array(1),
				array(1 => array(2))
			),
			array(
				array(
					array(
						'post_title' => 'test',
						'categories' => array('test2'),
						'metadata' => array(
							'test' => 'test2'
						)
					)
				),
				array(1),
				array(1 => array(2)),
				array(1 => array(
					'test' => 'test2'
				))
			),
			array(
				array(
					array(
						'post_title' => 'test',
						'categories' => array('test', 'test2/test3')
					)
				),
				array(1),
				array(1 => array(1,3))
			),
		);
	}

	/**
	 * @dataProvider providerTestCreatePosts
	 */
	function testCreatePosts($posts, $expected_post_ids, $expected_post_categories = false, $expected_metadata = false) {
		update_option('default_category', 1);
		wp_insert_category(array('slug' => 'test'));

		$this->assertEquals($expected_post_ids, $this->pf->create_posts($posts, array('test' => 1, 'test2' => 2, 'test2/test3' => 3)));

		if (is_array($expected_post_categories)) {
			foreach ($expected_post_categories as $post_id => $categories) {
				$this->assertEquals($categories, wp_get_post_categories($post_id));
			}
		}

		if (is_array($expected_metadata)) {
			foreach ($expected_metadata as $post_id => $metadata_info) {
				foreach ($metadata_info as $key => $value) {
					$this->assertEquals($value, get_post_meta($post_id, $key, true));
				}
			}
		}
	}

	function testCreate() {
		$pf = $this->getMock('PostFixtures', array('create_posts', 'create_categories', 'process_blog_options'));
		$pf->expects($this->once())->method('create_posts')->with('posts', 'processed');
		$pf->expects($this->once())->method('create_categories')->with('categories')->will($this->returnValue('processed'));
		$pf->expects($this->once())->method('process_blog_options')->with('options', 'processed');

		$pf->create(array('posts' => 'posts', 'categories' => 'categories', 'options' => 'options'));
	}

	function testRemove() {
		$pf = $this->getMock('PostFixtures', array('remove_all_posts', 'remove_all_categories'));

		$pf->expects($this->once())->method('remove_all_posts');
		$pf->expects($this->once())->method('remove_all_categories');

		$pf->remove();
	}

	function providerTestProcessBlogOptions() {
		return array(
			array(
				array(),
				array('test' => 'test')
			),
			array(
				array('test2' => 'test2'),
				array('test' => 'test', 'test2' => 'test2')
			),
			array(
				array('test' => false),
				array('test' => false)
			),
			array(
				array('test' => '${category:category}'),
				array('test' => '1')
			),
			array(
				array('test' => '${cat:category}'),
				array('test' => '1')
			),
		);
	}

	/**
	 * @dataProvider providerTestProcessBlogOptions
	 */
	function testProcessBlogOptions($data, $expected_fields) {
		update_option('test', 'test');

		$this->pf->process_blog_options($data, array('category' => 1));

		foreach ($expected_fields as $name => $value) {
			$this->assertEquals($value, get_option($name));
			if ($value === false) {
				$all_options = get_alloptions();
				$this->assertTrue(!isset($all_options[$name]));
			}
		}
	}

	function providerTestHandleUpdate() {
		return array(
			array(array(), false),
			array(array('is_ok' => true, 'data' => '{]'), false),
			array(array('is_ok' => true, 'data' => '{"test": "test"}'), true),
		);
	}

	/**
	 * @dataProvider providerTestHandleUpdate
	 */
	function testHandleUpdate($info, $should_succeed) {
		$pf = $this->getMock('PostFixtures', array('process_data', 'remove', 'create'));

		foreach (array('process_data', 'remove', 'create') as $method) {
			$pf->expects($this->{$should_succeed ? 'once' : 'never'}())->method($method);
		}

		$pf->handle_update($info);
	}

	function providerTestAdminInit() {
		return array(
			array(array(), false),
			array(array('pf' => false), false),
			array(array('pf' => array()), false),
			array(array('pf' => array('_nonce' => false)), false),
			array(array('pf' => array('_nonce' => 'post-fixtures')), true),
		);
	}

	/**
	 * @dataProvider providerTestAdminInit
	 */
	function testAdminInit($post, $should_update) {
		$_POST = $post;

		$pf = $this->getMock('PostFixtures', array('handle_update'));
		$pf->expects($this->{$should_update ? 'once' : 'never'}())->method('handle_update');

		_set_valid_nonce('post-fixtures', 'post-fixtures');

		$pf->admin_init();

		$this->assertTrue(!isset($_POST['pf']));
	}

	function testRecurseDirectories() {
		mkdir(vfsStream::url('root/dir/fixtures'), 0666, true);

		foreach (array(
			'dir/test.json',
			'dir/test.inc',
			'dir/fixtures/test',
			'dir/fixtures/test.json',
			'dir/fixtures/test.inc',
		) as $path) {
			$path = vfsStream::url("root/${path}");
			file_put_contents($path, $path);
		}

		$pf = $this->getMock('PostFixtures', array('wpcontent_path'));
		$pf->expects($this->any())->method('wpcontent_path')->will($this->returnValue(vfsStream::url('root')));

		$this->assertEquals(array(
			'dir/fixtures/test.json',
			'dir/fixtures/test.inc',
		), $pf->find_fixtures());
	}
}
