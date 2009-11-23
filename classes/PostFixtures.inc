<?php

class PostFixtures {
	var $messages;

	function init() {
		add_action('admin_menu', array(&$this, 'admin_menu'));
		add_action('admin_notices', array(&$this, 'admin_notices'));
	}

	function admin_notices() {
		if (!empty($this->messages)) { ?>
			<div class="fade updated">
				<?php foreach ($this->messages as $message) { ?>
					<p><?php echo $message ?></p>
				<?php } ?>
			</div>
		<?php }
	}

	function handle_update($info) {
		if (isset($info['is_ok'])) {
      $data = $this->parse_json(stripslashes($info['data']));
      if (!empty($data)) {
      	$data = $this->process_data($data);
      	$this->remove();
      	$this->create($data);
      	$this->messages[] = "New data set loaded into WordPress.";
      } else {
      	$this->messages[] = "Data is not valid JSON.";
      }
		}
	}

	function admin_menu() {
		global $plugin_page;

		add_submenu_page('tools.php', __('Post Fixtures', 'post-fixtures'), __('Post Fixtures', 'post-fixtures'), 'manage_options', 'post-fixtures', array(&$this, 'render_admin'));

		if ($plugin_page == 'post-fixtures') {
			wp_enqueue_script('jquery');
		}

		if (isset($_POST['pf'])) {
			if (isset($_POST['pf']['_nonce'])) {
				if (wp_verify_nonce($_POST['pf']['_nonce'], 'post-fixtures')) {
					$this->handle_update($_POST['pf']);
				}
			}
		}
	}

	function render_admin() { ?>
		<div class="wrap">
			<h2><?php _e('Post Fixtures', 'post-fixtures') ?></h2>

			<p>
				<?php _e('<strong>Post Fixtures</strong> is a developer tool for quickly setting up the WordPress database to test certain combinations of posts, categories, and meta data.', 'post-fixtures'); ?>
				<?php _e('<strong>You should not have this installed on a live site!</strong>', 'post-fixtures'); ?>
				<?php _e('This tool <strong>will delete all of your posts, categories, and metadata as necessary!</strong>', 'post-fixtures'); ?>
			</p>

			<form action="" method="post" id="post-fixtures-form">
				<input type="hidden" name="pf[_nonce]" value="<?php echo wp_create_nonce('post-fixtures') ?>" />
				<h3><?php _e('JSON data to load into the database:', 'post-fixtures') ?></h3>
				<textarea name="pf[data]" rows="20" style="width: 100%"></textarea>
				<label style="margin: 5px 0; display: block">
					<input type="checkbox" name="pf[is_ok]" value="yes" /> <?php _e('Yes, I want Post Fixtures to <strong>delete all of my data and load this data instead.</strong>', 'post-fixtures') ?><br />
				</label>
				<input type="submit" value="Load Provided Data" />
			</form>
			<script type="text/javascript">
				var form = jQuery('#post-fixtures-form').get(0);
				if (form) {
					jQuery(form).submit(function() {
						var checkbox = jQuery('input[name*=is_ok]', form).get(0);
						var ok = false;
						if (checkbox) {
							ok = checkbox.checked;
						}
						if (!ok) {
							alert('<?php _e('You must check the checkbox to approve this operation.', 'post-fixtures') ?>');
							return false;
						}
						return true;
					});
				}
			</script>
		</div>
	<?php }

	// data handling

	function parse_json($input) {
		if (($data = json_decode($input)) !== false) {
			if (is_array($data) || is_object($data)) {
				return (array)$data;
			}
		}
		return false;
	}

	function remove_all_posts() {
		if (is_array($posts = get_posts(array('numberposts' => '-1', 'post_status' => 'draft,pending,future,inherit,private,publish')))) {
			foreach ($posts as $post) {	wp_delete_post($post->ID); }
		}
	}

	function remove_all_categories() {
		foreach (get_all_category_ids() as $id) {
			wp_delete_category($id);
		}
	}

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

	function create($data) {
		$categories_by_slug = $this->create_categories($data['categories']);
		$this->create_posts($data['posts'], $categories_by_slug);
		$this->process_blog_options($data['options'], $categories_by_slug);
	}

	function remove() {
		$this->remove_all_posts();
		$this->remove_all_categories();
	}

	function process_blog_options($options, $categories) {
		$this->_category = $categories;
		foreach ($options as $option => $value) {
			$value = preg_replace_callback('#\$\{([^\}]+)\}#', array(&$this, '_process_blog_options_callback'), $value);
			update_option($option, $value);
		}
		unset($this->_category);
	}

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
