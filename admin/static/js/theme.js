$(document).ready(function(){
    $('.enable-theme').click(function() {
        var theme_name = $(this).attr('themename');
        var args = {action : 'enable', theme : theme_name};
        $.ajax({
            url : '/admin/theme/',
            data : args,
            type : 'post',
            dataType : 'json',
            success : function(result) {
                if (result['status'] == "success") {
                }
            },
            error: function(result) {
            }
        });
    });
});