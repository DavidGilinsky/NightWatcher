(function (blocks, element) {
    var el = element.createElement;
    blocks.registerBlockType('nightwatcher/latest-data', {
        title: 'NightWatcher Latest Data',
        icon: 'visibility',
        category: 'widgets',
        edit: function () {
            return el('div', {}, 'NightWatcher Latest Data block. Data will be shown on the site.');
        },
        save: function () {
            return null; // Server-side rendering
        }
    });
})(window.wp.blocks, window.wp.element);
