(function (blocks, element) {
    var el = element.createElement;
    blocks.registerBlockType('nightwatcher/simple', {
        title: 'NightWatcher',
        icon: 'visibility',
        category: 'widgets',
        edit: function () {
            return el('div', {}, 'NightWatcher');
        },
        save: function () {
            return null; // Server-side rendering
        }
    });
})(window.wp.blocks, window.wp.element);
