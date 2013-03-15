crawler.progress.connect(function(count) {
    document.getElementById('progress').textContent = 'Crawling ' + count + ' directories...';
});

crawler.finished.connect(function() {
    document.getElementById('progress').style.display = 'none';
    appendTreemap(document.getElementById('map'), JSON.parse(crawler.tree));
});
