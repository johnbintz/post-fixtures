<?php

class PostFixtures {
	function parse_json($input) {
		if (($data = json_decode($input)) !== false) {
			if (is_array($data) || is_object($data)) {
				return array_values((array)$data);
			}
		}
		return false;
	}

	function remove_all_posts() {
		if (is_array($posts = get_posts('nopaging=1'))) {
			foreach ($posts as $post) {	wp_delete_post($post->ID); }
		}
	}

	function remove_all_categories() {
		foreach (get_all_category_ids() as $id) {
			wp_delete_category($id);
		}
	}

	function process_data($data) {
		$posts = $data;
		$categories = array();
		foreach ($data as $post) {
			if (isset($post['categories'])) {
				$categories = array_merge($categories, $post['categories']);
			}
		}
		return compact('posts', 'categories');
	}

	function create_categories($categories) {
		$category_ids_by_slug = array();
		if (is_array($categories)) {
			foreach ($categories as $category) {
				$category_ids_by_slug[$category] = wp_insert_category(array('slug' => $category));
			}
		}
		return $category_ids_by_slug;
	}

	function create_posts($posts, $categories) {
		$post_ids_created = array();
		if (is_array($posts)) {
			foreach ($posts as $post) {
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
		$this->create_categories($data['categories']);
		$this->create_posts($data['posts'], $data['categories']);
	}
}
