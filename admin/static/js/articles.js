$(document).ready(function(){
    $('.delete-article').click(function() {
        article_to_delete = $(this).attr('articleid');
        $('#confirm-delete-article').modal();
    });
    
    $('.really-delete').click(function() {
        $.get('/admin/article/delete/' + article_to_delete + '/',
            function(result) {
                var obj = $.parseJSON(result);
                if (obj['status'] == "success") {
                    var selector_str = 'tr[articleid=\'' + article_to_delete + '\']';
                    $(selector_str).remove();
                    $('#confirm-delete-article').modal('toggle');
                }
            }
        );
    });
});