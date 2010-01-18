<?php

class FixtureBuilder {
	public $deferred_builds;
	private $current_object;

	function __construct() {
		$this->deferred_builds = array();
		$this->current_object = null;
	}

	function ensure_type($type) {
		if (!isset($this->deferred_builds[$type])) {
			$this->deferred_builds[$type] = array();
		}
	}

	function check_correct_type($name) {
		$ok = false;
		if (!empty($this->current_object)) {
			if ($name != $this->current_object['type']) {
				throw new LogicException("You can't call ${name} at this point.");
			}
			$ok = true;
		}
		return $ok;
	}

	function category($name) {
		if ($this->check_correct_type('category')) {
			$this->defer();
		}

		$this->current_object = array(
			'type' => 'category', 'name' => $name
		);

		return $this;
	}

	function post($title) {
		if ($this->check_correct_type('post')) {
			$this->defer();
		}

		$this->current_object = array(
			'type' => 'post', 'post' => array(
				'post_title' => $title,
				'post_type' => 'post'
			)
		);
		return $this;
	}

	function date($date) {
		$this->check_correct_type('post');

		$this->current_object['post']['post_date'] = $date;
		return $this;
	}

	function defer() {
		if (!empty($this->current_object)) {
			$this->ensure_type($this->current_object['type']);
			$this->{"defer_{$this->current_object['type']}"}($this->current_object);
		}
	}

	function build() {
		if (!empty($this->current_object)) {
			$this->ensure_type($this->current_object['type']);
			return $this->{"build_{$this->current_object['type']}"}($this->current_object);
		}
	}

	function categories($categories) {
		$this->check_correct_type('post');

		$this->current_object['post']['categories'] = explode(',', $categories);
		return $this;
	}

	function metadata($key, $value) {
		$this->check_correct_type('post');

		if (!isset($this->current_object['post'])) {
			$this->current_object['post'] = array();
		}
		$this->current_object['post']['metadata'][$key] = $value;
		return $this;
	}

	function defer_category($obj) {
		$this->deferred_builds['category'] = array_merge($this->deferred_builds['category'], explode(',', $obj['name']));
	}

	function defer_post($obj) {
		$this->deferred_builds['post'][] = $obj['post'];
	}

	function build_category($obj) {
		return PostFixtures::create_category($obj['name']);
	}

	function build_post($obj) {
		$post_obj = $obj['post'];
		foreach (array('categories', 'metadata') as $field) {
			unset($post_obj[$field]);
		}
		$post_id = PostFixtures::create_post($post_obj);
		if ($post_id != 0) {
			$category_ids  = array();
			if (isset($obj['post']['categories'])) {
				$category_ids_by_slug = array();
				foreach ($obj['post']['categories'] as $category) {
					$category_ids_by_slug = array_merge($category_ids_by_slug, PostFixtures::create_category($category));
				}
				$category_ids = array_values($category_ids_by_slug);
			}

			$metadata = array();
			if (isset($obj['post']['metadata'])) {
				foreach ($obj['post']['metadata'] as $key => $value) {
					PostFixtures::create_post_metadata($post_id, $key, $value);
				}
			}

			return get_post($post_id);
		}
		return false;
	}
}
