<?php
/*
Plugin Name: NightWatcher Simple Block with DB and REST API
Description: Displays "NightWatcher" in a block, provides a setup page to verify a dedicated MySQL database and user, creates a table, and receives data via REST API.
Version: 1.3
Author: NightWatcher Team
*/

if (!defined('ABSPATH')) exit;

// --- Block Registration ---
function nightwatcher_register_block() {
    if (!function_exists('register_block_type')) return;
    wp_register_script(
        'nightwatcher-block',
        plugins_url('block.js', __FILE__),
        ['wp-blocks', 'wp-element'],
        false,
        true
    );
    register_block_type('nightwatcher/simple', [
        'editor_script' => 'nightwatcher-block',
        'render_callback' => function() {
            return '<div>NightWatcher</div>';
        },
    ]);
}
add_action('init', 'nightwatcher_register_block');

// --- Admin Setup Page ---
function nightwatcher_admin_menu() {
    add_options_page('NightWatcher Setup', 'NightWatcher Setup', 'manage_options', 'nightwatcher-setup', 'nightwatcher_setup_page');
}
add_action('admin_menu', 'nightwatcher_admin_menu');

function nightwatcher_setup_page() {
    if (!current_user_can('manage_options')) return;
    $msg = '';
    $success = false;
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && check_admin_referer('nightwatcher_setup')) {
        $db_name = sanitize_text_field($_POST['db_name']);
        $db_user = sanitize_text_field($_POST['db_user']);
        $db_pass = sanitize_text_field($_POST['db_pass']);
        if ($db_name && $db_user && $db_pass) {
            $result = nightwatcher_verify_db_and_user($db_name, $db_user, $db_pass);
            $msg = $result['msg'];
            $success = $result['success'];
            if ($success) {
                update_option('nightwatcher_db_user', $db_user);
                update_option('nightwatcher_db_pass', $db_pass);
                update_option('nightwatcher_db_name', $db_name);
                // Create the data table
                nightwatcher_create_data_table($db_name, $db_user, $db_pass);
            }
        } else {
            $msg = 'Please provide database name, user, and password.';
        }
    }
    ?>
    <div class="wrap">
        <h1>NightWatcher Database Setup</h1>
        <p><strong>Instructions:</strong></p>
        <ol>
            <li>Using your hosting control panel or MySQL command line, <strong>create a new database</strong> (e.g., <code>nightwatcher_db</code>).</li>
            <li>Create a <strong>new MySQL user</strong> (e.g., <code>nightwatcher_user</code>) and <strong>grant that user ALL PRIVILEGES on the new database</strong>.</li>
            <li>Enter the database name, username, and password below to verify the setup.</li>
        </ol>
        <?php if (!empty($msg)) echo '<div style="margin:1em 0; color:' . ($success ? 'green' : 'red') . ';">' . esc_html($msg) . '</div>'; ?>
        <form method="post">
            <?php wp_nonce_field('nightwatcher_setup'); ?>
            <table class="form-table">
                <tr><th><label for="db_name">Database Name</label></th><td><input type="text" name="db_name" id="db_name" required></td></tr>
                <tr><th><label for="db_user">Database User</label></th><td><input type="text" name="db_user" id="db_user" required></td></tr>
                <tr><th><label for="db_pass">Database Password</label></th><td><input type="password" name="db_pass" id="db_pass" required></td></tr>
            </table>
            <p class="submit"><input type="submit" class="button-primary" value="Verify Database and User"></p>
        </form>
    </div>
    <?php
}

function nightwatcher_verify_db_and_user($db_name, $db_user, $db_pass) {
    $dbhost = DB_HOST;
    $mysqli = new mysqli($dbhost, $db_user, $db_pass, $db_name);
    if ($mysqli->connect_errno) {
        return ['success' => false, 'msg' => 'Connection failed: ' . $mysqli->connect_error];
    }
    // Try to create and drop a test table to check privileges
    $test_table = 'nw_test_' . wp_generate_password(8, false, false);
    $create = $mysqli->query("CREATE TABLE `$test_table` (id INT PRIMARY KEY) ENGINE=InnoDB");
    $drop = false;
    if ($create) {
        $drop = $mysqli->query("DROP TABLE `$test_table`");
    }
    $mysqli->close();
    if ($create && $drop) {
        return ['success' => true, 'msg' => 'Success! Database and user are properly configured.'];
    } elseif (!$create) {
        return ['success' => false, 'msg' => 'Connected, but user does not have CREATE TABLE privilege on the database.'];
    } else {
        return ['success' => false, 'msg' => 'Could not drop test table. Please check user privileges.'];
    }
}

function nightwatcher_create_data_table($db_name, $db_user, $db_pass) {
    $dbhost = DB_HOST;
    $mysqli = new mysqli($dbhost, $db_user, $db_pass, $db_name);
    if ($mysqli->connect_errno) return;
    $sql = "CREATE TABLE IF NOT EXISTS nightwatcher_data (
        id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
        datetime DATETIME NOT NULL,
        site_name VARCHAR(255) NOT NULL,
        latitude DOUBLE NOT NULL,
        longitude DOUBLE NOT NULL,
        mpsqa DOUBLE NOT NULL,
        temperature DOUBLE NOT NULL,
        pressure DOUBLE NOT NULL,
        humidity DOUBLE NOT NULL,
        PRIMARY KEY (id)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
    $mysqli->query($sql);
    $mysqli->close();
}

// --- REST API Endpoint ---
add_action('rest_api_init', function() {
    register_rest_route('nightwatcher/v1', '/submit', [
        'methods' => 'POST',
        'callback' => 'nightwatcher_receive_data',
        'permission_callback' => 'nightwatcher_api_auth',
        'args' => [
            'datetime' => ['required' => true, 'type' => 'string'],
            'site_name' => ['required' => true, 'type' => 'string'],
            'latitude' => ['required' => true, 'type' => 'number'],
            'longitude' => ['required' => true, 'type' => 'number'],
            'mpsqa' => ['required' => true, 'type' => 'number'],
            'temperature' => ['required' => true, 'type' => 'number'],
            'pressure' => ['required' => true, 'type' => 'number'],
            'humidity' => ['required' => true, 'type' => 'number'],
        ],
    ]);
});

function nightwatcher_api_auth($request) {
    // HTTP Basic Auth: check against stored DB user/pass
    if (!isset($_SERVER['PHP_AUTH_USER']) || !isset($_SERVER['PHP_AUTH_PW'])) {
        header('WWW-Authenticate: Basic realm="NightWatcher API"');
        return new WP_Error('rest_forbidden', 'Authentication required.', ['status' => 401]);
    }
    $db_user = get_option('nightwatcher_db_user');
    $db_pass = get_option('nightwatcher_db_pass');
    if ($_SERVER['PHP_AUTH_USER'] !== $db_user || $_SERVER['PHP_AUTH_PW'] !== $db_pass) {
        return new WP_Error('rest_forbidden', 'Invalid credentials.', ['status' => 403]);
    }
    return true;
}

function nightwatcher_receive_data($request) {
    $db_name = get_option('nightwatcher_db_name');
    $db_user = get_option('nightwatcher_db_user');
    $db_pass = get_option('nightwatcher_db_pass');
    $dbhost = DB_HOST;
    $mysqli = new mysqli($dbhost, $db_user, $db_pass, $db_name);
    if ($mysqli->connect_errno) {
        return new WP_Error('db_connect_error', 'Could not connect to NightWatcher DB.', ['status' => 500]);
    }
    $stmt = $mysqli->prepare("INSERT INTO nightwatcher_data (datetime, site_name, latitude, longitude, mpsqa, temperature, pressure, humidity) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    if (!$stmt) {
        $mysqli->close();
        return new WP_Error('db_prepare_error', 'Could not prepare statement.', ['status' => 500]);
    }
    $stmt->bind_param(
        'ssdddddd',
        $request['datetime'],
        $request['site_name'],
        $request['latitude'],
        $request['longitude'],
        $request['mpsqa'],
        $request['temperature'],
        $request['pressure'],
        $request['humidity']
    );
    $ok = $stmt->execute();
    $stmt->close();
    $mysqli->close();
    if ($ok) {
        return ['success' => true];
    } else {
        return new WP_Error('db_insert_error', 'Could not insert data.', ['status' => 500]);
    }
}
