<?php
/*
Plugin Name: NightWatcher API Receiver
Description: Receives and displays sky quality and environmental data from NightWatcher.
Version: 1.0
Author: NightWatcher Team
*/

if (!defined('ABSPATH')) exit;

class NightWatcher_API_Receiver {
    const OPTION_KEY = 'nightwatcher_latest_data';
    const REST_NAMESPACE = 'nightwatcher/v1';
    const REST_ROUTE = '/submit';

    public function __construct() {
        add_action('rest_api_init', [$this, 'register_api']);
        add_action('init', [$this, 'register_block']);
    }

    public static function activate() {
        global $wpdb;
        $table_name = $wpdb->prefix . 'nightwatcher_data';
        $charset_collate = $wpdb->get_charset_collate();
        require_once(ABSPATH . 'wp-admin/includes/upgrade.php');
        $sql = "CREATE TABLE IF NOT EXISTS $table_name (
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
        ) $charset_collate;";
        dbDelta($sql);
    }

    public function register_api() {
        register_rest_route(self::REST_NAMESPACE, self::REST_ROUTE, [
            'methods' => 'POST',
            'callback' => [$this, 'handle_submit'],
            'permission_callback' => [$this, 'authenticate_request'],
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
    }

    public function authenticate_request($request) {
        // Use Application Passwords authentication
        if (!is_user_logged_in()) {
            return new WP_Error('rest_forbidden', 'Authentication required.', ['status' => 401]);
        }
        $user = wp_get_current_user();
        if (!$user || !user_can($user, 'edit_posts')) {
            return new WP_Error('rest_forbidden', 'Insufficient permissions.', ['status' => 403]);
        }
        return true;
    }

    public function handle_submit($request) {
        global $wpdb;
        $table_name = $wpdb->prefix . 'nightwatcher_data';
        $data = [
            'datetime' => $request['datetime'],
            'site_name' => $request['site_name'],
            'latitude' => $request['latitude'],
            'longitude' => $request['longitude'],
            'mpsqa' => $request['mpsqa'],
            'temperature' => $request['temperature'],
            'pressure' => $request['pressure'],
            'humidity' => $request['humidity'],
        ];
        $wpdb->insert($table_name, [
            'datetime' => $data['datetime'],
            'site_name' => $data['site_name'],
            'latitude' => $data['latitude'],
            'longitude' => $data['longitude'],
            'mpsqa' => $data['mpsqa'],
            'temperature' => $data['temperature'],
            'pressure' => $data['pressure'],
            'humidity' => $data['humidity'],
        ]);
        return ['success' => true, 'data' => $data];
    }

    public function register_block() {
        if (!function_exists('register_block_type')) return;
        wp_register_script(
            'nightwatcher-block',
            plugins_url('block.js', __FILE__),
            ['wp-blocks', 'wp-element', 'wp-editor'],
            filemtime(plugin_dir_path(__FILE__) . 'block.js')
        );
        register_block_type('nightwatcher/latest-data', [
            'editor_script' => 'nightwatcher-block',
            'render_callback' => [$this, 'render_block'],
        ]);
    }

    public function render_block() {
        global $wpdb;
        $table_name = $wpdb->prefix . 'nightwatcher_data';
        $data = $wpdb->get_row("SELECT * FROM $table_name ORDER BY datetime DESC, id DESC LIMIT 1", ARRAY_A);
        if (!$data) return '<div>No NightWatcher data received yet.</div>';
        return sprintf(
            '<div class="nightwatcher-data">
                <strong>Site:</strong> %s<br>
                <strong>Date/Time:</strong> %s<br>
                <strong>Latitude:</strong> %s<br>
                <strong>Longitude:</strong> %s<br>
                <strong>mpsqa:</strong> %s<br>
                <strong>Temperature:</strong> %s °C<br>
                <strong>Pressure:</strong> %s hPa<br>
                <strong>Humidity:</strong> %s%%
            </div>',
            esc_html($data['site_name']),
            esc_html($data['datetime']),
            esc_html($data['latitude']),
            esc_html($data['longitude']),
            esc_html($data['mpsqa']),
            esc_html($data['temperature']),
            esc_html($data['pressure']),
            esc_html($data['humidity'])
        );
    }
}

register_activation_hook(__FILE__, ['NightWatcher_API_Receiver', 'activate']);
new NightWatcher_API_Receiver();
