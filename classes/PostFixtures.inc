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
}
