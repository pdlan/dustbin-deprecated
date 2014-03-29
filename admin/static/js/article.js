var article_to_delete = '';
var tags = new Array();

$('#new-tag').click(function() {
    var new_tag = $('#new-tag-input').val();
    var found = false;
    for (var i = 0; i < tags.length; ++i) {
        if (tags[i] == new_tag) {
            found = true;
        }
    }
    if (!found) {
        tags.push(new_tag);
        refresh_tags();
    }
});

function delete_tag_click() {
    var tag = $(this).attr('tag');
    for (var i = 0; i < tags.length; ++i) {
        if (tags[i] == tag) {
            tags.splice(i, 1);
        }
    }
    refresh_tags();
}

function refresh_tags() {
    $('#tags-show').html('');
    $('#tags').val('');
    for (var i = 0; i < tags.length; ++i) {
        var tag = tags[i];
        var html_show =
            '<span class="label label-default tag">' + tag +
            ' <span class="delete-tag" tag="' + tag +
            '"><i class="fa fa-times"></i></span></span>';
        $('#tags-show').append(html_show);
        var tags_val = $('#tags').val();
        if (i == 0) {
            $('#tags').val(tags_val + tag);
        } else {
            $('#tags').val(tags_val + ',' + tag);
        }
        $('.delete-tag').click(delete_tag_click);
    }
}

function parse_tags(tags_str) {
    tags = tags_str.split(',');
}

if ($('#tags').val() != '') {
    parse_tags($('#tags').val());
}
    
refresh_tags();