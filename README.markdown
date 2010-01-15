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

Fixtures for **Post Fixtures** are written in JSON. Save them with the theme/plugin you're developing and
copy/paste them into **Post Fixtures** when you need to test specific features.

### What's Supported?

As of the current release on GitHub, the following WordPress features are supported:

* Posts
	* Most post data found in the `posts` table
	* Post metadata with serialization
* Categories
* Blog options with serialization and deletion

## Contributing

The best way to contribute is forking the project on GitHub and sending a Pull Request. Be sure to include
proper unit tests for any extra code you write. If you've found a bug or have a new feature you'd like to see,
create a new Issue.

## Example Fixture

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
			}
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
