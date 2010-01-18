<?php
/*
Plugin Name: Post Fixtures
Plugin URI: http://www.coswellproductions.com/wordpress/wordpress-plugins/
Description: Tear down and build up well crafted posts, categories, meta data, and other WordPress data in a test environment.
Version: 0.1
Author: John Bintz
Author URI: http://www.coswellproductions.com/wordpress/

Copyright 2009 John Bintz  (email : john@coswellproductions.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

function __post_fixtures_plugins_loaded() {
	if (version_compare(PHP_VERSION, '5.0.0') === 1) {
		foreach (glob(dirname(__FILE__) . '/classes/*.inc') as $file) {
			require_once($file);
		}
		$post_fixtures = new PostFixtures();
		$post_fixtures->init();
	} else {
		add_action('admin_notices', '__post_fixtures_admin_notices');
	}
}

function __post_fixtures_admin_notices() {
	deactivate_plugins(plugin_basename(__FILE__)); ?>
	<div class="updated fade"><p><?php _e('You need to be running at least PHP 5 to use Post Fixtures. Plugin <strong>not</strong> activated.') ?></p></div>
<?php }

add_action('plugins_loaded', '__post_fixtures_plugins_loaded');
