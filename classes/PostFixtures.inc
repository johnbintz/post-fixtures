<?php

/**
 * Set up fresh WordPress data quickly and easily.
 */
class PostFixtures {
	var $messages;

	/**
	 * Initialize the plugin.
	 */
	// @codeCoverageIgnoreStart
	function init() {
		foreach (array(
			'admin_init' => array(),
			'admin_menu' => array(),
			'admin_notices' => array(),
			'admin_enqueue_scripts' => array(10, 1),
		) as $method => $additional_params) {
			call_user_func_array(
				'add_action',
				array_merge(
					array($method, array(&$this, $method)),
					$additional_params
				)
			);
		}
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Print any admin notices.
	 */
	// @codeCoverageIgnoreStart
	function admin_notices() {
		if (!empty($this->messages)) { ?>
			<div class="fade updated">
				<?php foreach ($this->messages as $message) { ?>
					<p><?php echo $message ?></p>
				<?php } ?>
			</div>
		<?php }
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Handle an update.
	 * @param array $info The post info.
	 */
	function handle_update($info) {
		if (isset($info['is_ok'])) {
			if (strpos($info['data'], 'file:') === 0) {
				$this->handle_load_file(substr($info['data'], 5));
			} else {
				$this->handle_json($info['data']);
			}
		}
	}

	function handle_load_file($file) {
		$file = realpath($this->wpcontent_path() . '/' . $file);
		if (file_exists($file)) {
			switch (pathinfo($file, PATHINFO_EXTENSION)) {
				case "json":
					$this->handle_json(file_get_contents($file));
					break;
				case "inc";
					$this->remove();
					$builder = new FixtureBuilder();
					include($file);
					$builder->finalize();
					break;
			}
		}
	}

	function handle_json($input) {
		$data = $this->parse_json(stripslashes($input));
		if (!empty($data)) {
			$data = $this->process_data($data);
			$this->remove();
			$this->create($data);
			$this->messages[] = __("New data set loaded into WordPress.", 'post-fixtures');
		} else {
			$this->messages[] = __("Data is not valid JSON.", 'post-fixtures');
		}
	}

	/**
	 * Handle the admin_menu action.
	 */
	// @codeCoverageIgnoreStart
	function admin_menu() {
		global $plugin_page;

		$this->hook_suffix = add_submenu_page('tools.php', __('Post Fixtures', 'post-fixtures'), __('Post Fixtures', 'post-fixtures'), 'manage_options', 'post-fixtures', array(&$this, 'render_admin'));
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Handle the admin_init action.
	 * This is where the processing happens.
	 */
	function admin_init() {
		if (isset($_POST['pf']['_nonce'])) {
			if (wp_verify_nonce($_POST['pf']['_nonce'], 'post-fixtures')) {
				$this->handle_update($_POST['pf']);
			}
		}
		unset($_POST['pf']);
	}

	/**
	 * Handle the admin_enqueue_scripts action.
	 * @param string $hook_suffix The hook suffix for the page being loaded.
	 */
	// @codeCoverageIgnoreStart
	function admin_enqueue_scripts($hook_suffix) {
		if ($this->hook_suffix == $hook_suffix) {
			wp_enqueue_script('jquery');
			wp_enqueue_style('post-fixtures', plugin_dir_url(dirname(__FILE__)) . 'style.css');
		}
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Render the admin page.
	 */
	// @codeCoverageIgnoreStart
	function render_admin() {
		$fixtures = $this->find_fixtures();

		include(dirname(__FILE__) . '/partials/admin.inc');
	}

	function wpcontent_path() { return ABSPATH . '/wp-content'; }
	// @codeCoverageIgnoreEnd

	function find_fixtures() {
		$this->fixtures = array();

		$this->_find_fixtures_recurse($this->wpcontent_path());

		foreach ($this->fixtures as &$path) {
			$path = str_replace($this->wpcontent_path() . '/', '', $path);
		}

		return $this->fixtures;
	}

	function _find_fixtures_recurse($dir) {
		$queue = array();
		$dh = opendir($dir);
		$is_fixture_dir = (basename($dir) === 'fixtures');
		while ($file = readdir($dh)) {
			if ($file[0] != '.') {
				$target = $dir . '/' . $file;
				if ($is_fixture_dir) {
					if (is_file($target)) {
						if (preg_match('#\.(inc|json)$#', $target) > 0) {
							$this->fixtures[] = $target;
						}
					}
				} else {
					if (is_dir($target)) {
						$queue[] = $target;
					}
				}
			}
		}
		closedir($dh);

		foreach ($queue as $dir) {
			$this->_find_fixtures_recurse($dir);
		}
	}

	// data handling

	/**
	 * Parse incoming JSON data.
	 * @param string $input The JSON data to parse.
	 * @return array|false An array of JSON data, or false if invalid.
	 */
	function parse_json($input) {
		if (($data = json_decode($input)) !== false) {
			if (is_array($data) || is_object($data)) {
				return (array)$data;
			}
		}
		return false;
	}

	/**
	 * Remove all posts in the database.
	 * This is done via the WP interface so that associated comments are also deleted.
	 */
	function remove_all_posts() {
		if (is_array($posts = get_posts(array('numberposts' => '-1', 'post_status' => 'draft,pending,future,inherit,private,publish')))) {
			foreach ($posts as $post) {	wp_delete_post($post->ID); }
		}
	}

	/**
	 * Remove all categories in the database.
	 */
	function remove_all_categories() {
		foreach (get_all_category_ids() as $id) {
			wp_delete_category($id);
		}
	}

	/**
	 * Process the provided data and assemble a list of objects to create in the database.
	 * @param array $data The data to parse.
	 * @return array The list of objects to create.
	 */
	function process_data($data) {
		$posts = $categories = $options = array();

		foreach ($data as $type => $info) {
			switch ($type) {
				case 'posts':
					$posts = $info;
					foreach ($posts as $post) {
						$post = (array)$post;
						if (isset($post['categories'])) {
							$categories = array_merge($categories, $post['categories']);
						}
					}
					break;
				case 'options':
					$options = $info;
					break;
			}
		}
		$categories = array_unique($categories);
		return compact('posts', 'categories', 'options');
	}

	/**
	 * The categories to create.
	 * Categories are passed as name/name/name, with parent -> child relationships being constructed as necessary.
	 */
	function create_categories($categories) {
		$category_ids_by_slug = array();
		if (is_array($categories)) {
			foreach ($categories as $category) {
				$category_ids_by_slug = array_merge($category_ids_by_slug, $this->create_category($category));
			}
		}
		return $category_ids_by_slug;
	}

	function create_category($category) {
		$category_ids_by_slug = array();
		$nodes = explode('/', $category);
		$parent = 0;
		$joined_nodes = array();
		foreach ($nodes as $node) {
			$joined_nodes[] = $node;
			$parent = $category_ids_by_slug[implode('/', $joined_nodes)] = wp_insert_category(array('cat_name' => $node, 'category_nicename' => $node, 'category_parent' => $parent));
		}
		return $category_ids_by_slug;
	}

	/**
	 * The posts to create.
	 * Post data is passed in just as if you were using wp_insert_post().
	 * Categories are assigned using slug names separated by commas.
	 */
	function create_posts($posts, $categories) {
		$post_ids_created = array();
		if (is_array($posts)) {
			foreach ($posts as $post) {
				$id = $this->create_post($post);
				if ($id != 0) {
					$post_ids_created[] = $id;
					if (isset($post['categories'])) {
						$all_categories = array();
						foreach ($post['categories'] as $slug) {
							if (isset($categories[$slug])) {
								$all_categories[] = $categories[$slug];
							}
						}
						wp_set_post_categories($id, $all_categories);
					} else {
						wp_set_post_categories($id, array(get_option('default_category')));
					}

					if (isset($post['metadata'])) {
						foreach ($post['metadata'] as $key => $value) {
							$this->create_post_metadata($id, $key, $value);
						}
					}

					if (isset($post['tags'])) {
						$this->create_post_tags($id, $post['tags']);
					}
				}
			}
		}
		return $post_ids_created;
	}

	function create_post($post) {
		$post = (array)$post;
		if (!isset($post['post_status'])) {
			$post['post_status'] = 'publish';
		}
		return wp_insert_post($post);
	}

	function create_post_metadata($post_id, $key, $value) {
		update_post_meta($post_id, $key, $value);
	}

	function create_post_tags($post_id, $tags) {
		wp_set_post_tags($post_id, $tags);
	}

	function set_post_categories($post_id, $categories) {
		wp_set_post_categories($post_id, $categories);
	}

	/**
	 * Create everything from the provided data.
	 * @param array $data The data to use in creation.
	 */
	function create($data) {
		$categories_by_slug = $this->create_categories($data['categories']);
		$this->create_posts($data['posts'], $categories_by_slug);
		$this->process_blog_options($data['options'], $categories_by_slug);
	}

	/**
	 * Remove everything from the WordPress database that Post Fixures handles.
	 */
	function remove() {
		$this->remove_all_posts();
		$this->remove_all_categories();
	}

	/**
	 * Update the provided blog options.
	 * Option values can have other values injected into them. Currently only category slug names are available.
	 * @param array $options The options to set or unset. Pass in `false` to unset them.
	 * @param array $categories The list of categories to work with in string replacement.
	 */
	function process_blog_options($options, $categories) {
		$this->_category = $categories;
		foreach ($options as $option => $value) {
			if ($value === false) {
				delete_option($option);
			} else {
				$value = preg_replace_callback('#\$\{([^\}]+)\}#', array(&$this, '_process_blog_options_callback'), $value);
				update_option($option, $value);
			}
		}
		unset($this->_category);
	}

	/**
	 * Callback for process_blog_options
	 * @param array $matches Matches from preg_replace_callback
	 * @return string The replacement value for the match.
	 */
	function _process_blog_options_callback($matches) {
		$value = $matches[0];
		$parts = explode(':', $matches[1]);
		if (count($parts) > 1) {
			$source = strtolower(array_shift($parts));
			switch ($source) {
				case 'cat': $source = 'category'; break;
			}
			if (count($parts) == 1) {
				$index = reset($parts);
				if (isset($this->{"_${source}"})) {
					if (isset($this->{"_${source}"}[$index])) {
						$value = $this->{"_${source}"}[$index];
					}
				}
			}
		}

		return $value;
	}
}
