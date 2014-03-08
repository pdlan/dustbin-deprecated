<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width" />
    <title></title>
    <link rel="stylesheet" href="/static/clean.css" type="text/css" />
    <link rel="alternate" type="application/rss+xml" href="/feed.xml" title="{{site_name}}" />
</head>
<body>
    <div id="wrapper" class="yue">
        <header id="header">
            <a href="{{site_url}}" id="site_name">{{site_name}}</a>
            <nav>
                <a href="/archives/">Archives</a>
            </nav>
        </header>
        <div id="content">
        {{block_content}}
        </div>
        <footer id="footer">
            Powered by <a href="http://github.com/pdlan/dustbin">Dustbin</a>
            &
            Theme <a href="https://github.com/whtsky/catsup-theme-clean">Clean</a> by <a href="http://whouz.com">whtsky</a>
        </footer>
    </div>
</body>
</html>