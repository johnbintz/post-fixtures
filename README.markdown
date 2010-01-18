# Post Fixtures for WordPress

**Post Fixtures** let you quickly tear down and set up test environments within your development WordPress environment.
This allows you to create post, category, and options configurations to test specific issues and features of your themes and plugins.

## Installing

### From GitHub

Clone the repository into your `wp-content/plugins` directory. Activate it like a normal plugin.

## Using

**Post Fixtures** places a new menu item under *Tools* called *Post Fixtures*.
When you visit the page, you'll see a large textarea in which to copy and paste your JSON fixture data.
Submitting the form with valid JSON data will cause your posts and categories to be deleted & overwritten,
and any options provided to be overwritten or deleted.

## Creating Fixtures

Fixtures for **Post Fixtures** are written in either JSON or PHP.

### JSON

JSON fixtures can be saved anywhere with the theme/plugin you're developing and then copied and pasted into **Post Fixtures** when you need to test specific features.

### PHP

PHP fixtures are saved in a directory within your theme or plugin called `fixtures` with the extension `.inc`.
See the example fixture under `fixtures/php-ficture.inc` as well as the example below.

### What's Supported?

As of the current release on GitHub, the following WordPress features are supported:

* Posts
	* Most post data found in the `posts` table
	* Post metadata with serialization
* Categories
* Tags
* Blog options with serialization and deletion

## Contributing

The best way to contribute is forking the project on GitHub and sending a Pull Request. Be sure to include
proper unit tests for any extra code you write. If you've found a bug or have a new feature you'd like to see,
create a new Issue.

## Example Fixture

### JSON

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

### PHP

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
