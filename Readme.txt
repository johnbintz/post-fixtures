=== Post Fixtures ===
Contributors: johncoswell
Donate link: http://www.coswellproductions.com/wordpress/wordpress-plugins
Tags: admin, developer, database
Requires at least: 2.8
Tested up to: 2.9.1
Stable tag: 0.2.3

Post Fixtures let you quickly tear down and set up test environments within your development WordPress environment.

== Description ==

Post Fixtures let you quickly tear down and set up test environments within your development WordPress environment.
This allows you to create post, category, and options configurations to test specific issues and features of your themes and plugins.

**Post Fixtures** places a new menu item under *Tools* called *Post Fixtures*.
When you visit the page, you'll see a large textarea in which to copy and paste your JSON fixture data.
Submitting the form with valid JSON data will cause your posts and categories to be deleted & overwritten,
and any options provided to be overwritten or deleted.

As of the current release on GitHub, the following WordPress features are supported:

* Posts
	* Most post data found in the `posts` table
	* Post metadata with serialization
* Categories
* Tags
* Blog options with serialization and deletion

== Installation ==

Activate it like any other plugin. **Post Fixtures** requires PHP 5 or above.

== Frequently Asked Questions ==

= What are the data formats accepted? =

**JSON**

<pre>
{
	"posts": [
		{
			"post_date": "2010-01-01 9:00am",
			"post_title": "This is a sample post",
			"post_content": "This is the post's content",
			"categories": [ "Top Level 1/Sub Category 1", "Top Level 2/Sub Category 2" ],
			"metadata": {
				"field-1": "value 1",
				"field-2": {
					"a_complex": "field",
					"with": [
						"lots", "of", "nested", "datatypes"
					]
				}
			},
			"tags": "tag 1,tag 2"
		},
		{
			"post_date": "2010-01-01 10:00am",
			"post_title": "This is the second sample post",
			"post_content": "This is the second post's content",
			"categories": [ "Top Level 1/Sub Category 2", "Top Level 2/Sub Category 2" ]
		}
	],
	"options": {
		"an-option-to-set": "simple-string",
		"an-option-to-serialize": {
			"this": "is a hash"
		},
		"an-option-to-delete": false
	}
}
</pre>

**PHP**

<pre>
// build an object immediately, and get the new post's ID
$post_id = $builder->post('This is a sample post')
                   ->date('2010-01-01 9:00am')
                   ->content("This is the post's content")
                   ->categories("Top Level 1/Sub Category 1,Top Level 2/Sub Category 2")
                   ->metadata('field-1', 'value-1')
                   ->metadata('field-2', array(
                       'a_complex' => 'field',
                       'with' => array(
                       	'lots', 'of', 'nested', 'datatypes'
                       )
                     ))
                   ->tags('tag 1,tag 2')->build();

// build and object at the end, if order doesn't matter
$builder->post('This is the second sample post')
        ->date('2010-01-01 10:00am')
        ->content("This is the second post's content")
        ->categories("Top Level 1/Sub Category 2,Top Level 2/Sub Category 2")->defer();

// convenience wrapper around options setting
$builder->option('an-option-to-set', 'simple-string')
        ->option('an-option-to-serialize', array('this' => 'is a hash'))
        ->option('an-option-to-delete', false);
</pre>

== Changelog ==

= 0.2.3 =

* Clear cache after running fixtures. Needed for persistent caches.

= 0.2.2 =

* Bugfix for immediate build generation and sub-category addition when adding posts.

= 0.2.1 =

* Bugfix for multiple generated nested categories.

= 0.2 =

* Initial release on WordPress Plugins site.

== Upgrade Notice ==

= 0.2.3 =

* Clear cache after running fixtures. Needed for persistent caches.

= 0.2.2 =

* Bugfix for immediate build generation and sub-category addition when adding posts.

= 0.2.1 =

* Bugfix for multiple generated nested categories.

= 0.2 =

* Initial release.
