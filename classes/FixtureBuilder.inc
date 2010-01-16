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
			$this->{"build_{$this->current_object['type']}"}($this->current_object);
		}
	}

	function build_category($obj) {
		$this->deferred_builds['category'] = array_merge($this->deferred_builds['category'], explode('/', $obj['name']));
	}

	function build_post($obj) {
		$this->deferred_builds['post'][] = $obj['post'];
	}
}
