<div class="wrap">
	<h2><?php _e('Post Fixtures', 'post-fixtures') ?></h2>

	<p>
		<?php _e('<strong>Post Fixtures</strong> is a developer tool for quickly setting up the WordPress database to test certain combinations of posts, categories, and meta data.', 'post-fixtures'); ?>
		<?php _e('<strong>You should not have this installed on a live site!</strong>', 'post-fixtures'); ?>
		<?php _e('This tool <strong>will delete all of your posts, categories, and metadata as necessary!</strong>', 'post-fixtures'); ?>
	</p>

	<form action="" method="post" id="post-fixtures-form">
		<input type="hidden" name="pf[_nonce]" value="<?php echo wp_create_nonce('post-fixtures') ?>" />
		<div style="overflow: hidden">
			<div style="float: left;display: inline;width: 60%">
				<h3><?php _e('JSON data to load into the database:', 'post-fixtures') ?></h3>
				<textarea name="pf[data]" rows="20" style="width: 100%"></textarea>
			</div>
			<div style="float: left;display: inline;width: 38%;margin-left:2%">
				<h3><?php _e('Pre-built fixtures:', 'post-fixtures') ?></h3>
				<ul id="pre-built-fixture-holder">
					<?php foreach ($fixtures as $fixture) { ?>
						<li><a href="<?php echo esc_attr($fixture) ?>"><?php echo esc_html($fixture) ?></a></li>
					<?php } ?>
				</ul>
			</div>
		</div>
		<label>
			<input type="checkbox" name="pf[is_ok]" value="yes" /> <?php _e('Yes, I want Post Fixtures to <strong>delete all of my data and load this data instead.</strong>', 'post-fixtures') ?><br />
		</label>
		<div id="post-fixtures-additional-options">
		</div>
		<input type="submit" value="Load Provided Data" />
	</form>
	<script type="text/javascript">
		(function($) {
			$('input[name*=is_ok]').change(function() {
				$('#post-fixtures-additional-options')[$('input[name*=is_ok]:checked').val() ? 'show' : 'hide'](250);
			});

			$('#post-fixtures-additional-options').hide();
			$('input[name*=is_ok]').get(0).checked = false;

			$('#pre-built-fixture-holder a').click(function() {
				$('textarea[name*=data]').val('file:' + $(this).attr('href'));
				return false;
			});

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
