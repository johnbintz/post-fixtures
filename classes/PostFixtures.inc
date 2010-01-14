<?php

/**
 * Set up fresh WordPress data quickly and easily.
 */
class PostFixtures {
	var $messages;

	/**
	 * Initialize the plugin.
	 */
	function init() {
		add_action('admin_menu', array(&$this, 'admin_menu'));
		add_action('admin_notices', array(&$this, 'admin_notices'));
		add_action("admin_enqueue_scripts", array(&$this, 'admin_enqueue_scripts'), 10, 1);
	}

	/**
	 * Print any admin notices.
	 */
	function admin_notices() {
		if (!empty($this->messages)) { ?>
			<div class="fade updated">
				<?php foreach ($this->messages as $message) { ?>
					<p><?php echo $message ?></p>
				<?php } ?>
			</div>
		<?php }
	}

	/**
	 * Handle an update.
	 * @param array $info The post info.
	 */
	function handle_update($info) {
		if (isset($info['is_ok'])) {
      $data = $this->parse_json(stripslashes($info['data']));
      if (!empty($data)) {
      	$data = $this->process_data($data);
      	$this->remove();
      	$this->create($data);
      	$this->messages[] = __("New data set loaded into WordPress.", 'post-fixtures');
      } else {
      	$this->messages[] = __("Data is not valid JSON.", 'post-fixtures');
      }
		}
	}

	/**
	 * Handle the admin_menu action.
	 */
	function admin_menu() {
		global $plugin_page;

		$this->hook_suffix = add_submenu_page('tools.php', __('Post Fixtures', 'post-fixtures'), __('Post Fixtures', 'post-fixtures'), 'manage_options', 'post-fixtures', array(&$this, 'render_admin'));

		if (isset($_POST['pf'])) {
			if (isset($_POST['pf']['_nonce'])) {
				if (wp_verify_nonce($_POST['pf']['_nonce'], 'post-fixtures')) {
					$this->handle_update($_POST['pf']);
				}
			}
		}
	}

	/**
	 * Handle the admin_enqueue_scripts action.
	 * @param string $hook_suffix The hook suffix for the page being loaded.
	 */
	function admin_enqueue_scripts($hook_suffix) {
		if ($this->hook_suffix == $hook_suffix) {
			wp_enqueue_script('jquery');
			wp_enqueue_style('post-fixtures', plugin_dir_url(dirname(__FILE__)) . 'style.css');
		}
	}

	/**
	 * Render the admin page.
	 */
	function render_admin() {
		include(dirname(__FILE__) . '/partials/admin.inc');
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
				$nodes = explode('/', $category);
				$parent = 0;
				$joined_nodes = array();
				foreach ($nodes as $node) {
					$joined_nodes[] = $node;
					$parent = $category_ids_by_slug[implode('/', $joined_nodes)] = wp_insert_category(array('cat_name' => $node, 'slug' => $node, 'category_parent' => $parent));
				}
			}
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
				$post = (array)$post;
				if (!isset($post['post_status'])) {
					$post['post_status'] = 'publish';
				}
				$id = wp_insert_post($post);
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
							update_post_meta($id, $key, $value);
						}
					}
				}
			}
		}
		return $post_ids_created;
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
