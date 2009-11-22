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
}
