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
		(function($) {
			$('#post-fixtures-form').submit(function() {
				var checkbox = $('input[name*=is_ok]', this.target).get(0);
				var ok = false
				if (checkbox) {
					ok = checkbox.checked;
				}
				if (!ok) {
					alert('<?php _e('You must check the checkbox to approve this operation.', 'post-fixtures') ?>');
				}
				return ok;
			});
		}(jQuery));
	</script>
</div>
