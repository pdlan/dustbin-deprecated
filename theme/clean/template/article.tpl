<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    
    <meta name="viewport" content="width=device-width" />
    <title>{{title}} | {{site_name}}</title>
    <link rel="stylesheet" href="/static/clean.css" type="text/css" />
    <link rel="alternate" type="application/rss+xml" href="/feed.xml" title="{{site_name}}" />
    <link rel="canonical" href=""/>
</head>
<body>
    <div id="wrapper" class="yue">
        <header id="header">
            <a href="{{site_url}}" id="site_name">{{site_name}}</a>
            <nav>
                <a href="/archive/">Archives</a>
                
            </nav>
        </header>
        <div id="content">
        
<div class="post">
    <h1 id="title">{{title}}</h1>
    <div class="date">
    {{date}}
    </div>
    <article>
    {{content}}
    </article>
    
    <div class="tags">
        Tagged in :
        {{#tags}}
        <a href="/tag/{{name}}">{{name}}</a>
        {{/tags}}
        
    </div>
            <div id="disqus_thread"></div>
            <script type="text/javascript">
				var disqus_title = "";
                (function() {
                    var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;
                    dsq.src = 'http://{{disqus_shortname}}.disqus.com/embed.js';
                    (document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);
                })();
            </script>
    </div>

        </div>
        <footer id="footer">
            Powered by <a href="http://github.com/pdlan/dustbin">Dustbin</a>
            &
            Theme <a href="https://github.com/whtsky/catsup-theme-clean">Clean</a> by <a href="http://whouz.com">whtsky</a>
        </footer>
    </div>
</body>
</html>