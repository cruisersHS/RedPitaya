(function(SM, $, undefined) {
    SM.max_rate_1ch = [125e6,125e6];
    SM.max_rate_2chs = [125e6,125e6];

    SM.max_SD_rate_1ch = [125e6,125e6];
    SM.max_SD_rate_2chs = [125e6,125e6];

    SM.max_rate_devider_1ch = [2.0, 4.0];
    SM.max_rate_devider_2chs = [4.0, 8.0];

    SM.max_SD_rate_devider_1ch = [12.0 , 24.0];
    SM.max_SD_rate_devider_2chs = [24.0 , 48.0];
    

    SM.updateMaxLimits = function(model) {
        if (model !== undefined) {
            if (SM.rp_model == "") {
                if (model.value == "Z10") {
                    SM.rp_model = model.value;
                    var max_possible_rate = 125e6;
                    SM.ss_full_rate =  max_possible_rate;
                    SM.max_rate_1ch      = [max_possible_rate / SM.max_rate_devider_1ch[0] , max_possible_rate /SM.max_rate_devider_1ch[1]];
                    SM.max_rate_2chs     = [max_possible_rate / SM.max_rate_devider_2chs[0] , max_possible_rate / SM.max_rate_devider_2chs[1]];
                    
                    SM.max_SD_rate_1ch   = [max_possible_rate / SM.max_SD_rate_devider_1ch[0] , max_possible_rate /SM.max_SD_rate_devider_1ch[1]];
                    SM.max_SD_rate_2chs  = [max_possible_rate / SM.max_SD_rate_devider_2chs[0] , max_possible_rate / SM.max_SD_rate_devider_2chs[1]];
                    $("#SS_RATE").val(max_possible_rate);
                }

                if (model.value == "Z20") {
                    SM.rp_model = model.value;
                    var max_possible_rate = 122.88e6;
                    SM.ss_full_rate =  max_possible_rate;
                    SM.max_rate_1ch      = [max_possible_rate / SM.max_rate_devider_1ch[0] , max_possible_rate /SM.max_rate_devider_1ch[1]];
                    SM.max_rate_2chs     = [max_possible_rate / SM.max_rate_devider_2chs[0] , max_possible_rate / SM.max_rate_devider_2chs[1]];
                    
                    SM.max_SD_rate_1ch   = [max_possible_rate / SM.max_SD_rate_devider_1ch[0] , max_possible_rate /SM.max_SD_rate_devider_1ch[1]];
                    SM.max_SD_rate_2chs  = [max_possible_rate / SM.max_SD_rate_devider_2chs[0] , max_possible_rate / SM.max_SD_rate_devider_2chs[1]];
                    $("#SS_RATE").val(max_possible_rate);
                }
                
                SM.updateLimits();
            }
        }
        
    };

    SM.updateLimits = function() {
        var rate = 125e6;
        var rate_devider = 1;
        
        var use_net = $("#SS_USE_NET").prop('checked');
        var channel_mode = $("#SS_CHANNEL option:selected").val();
        var resolution = $("#SS_RESOLUTION option:selected").val();

        if (SM.rp_model == "Z10") {
            if (use_net) {
                if (channel_mode != 3) {
                    rate = SM.max_rate_1ch[resolution-1];
                    rate_devider = SM.max_rate_devider_1ch[resolution-1];
                }else{
                    rate = SM.max_rate_2chs[resolution-1];
                    rate_devider = SM.max_rate_devider_2chs[resolution-1];
                }
            }else {
                if (channel_mode != 3) {
                    rate = SM.max_SD_rate_1ch[resolution-1];
                    rate_devider = SM.max_SD_rate_devider_1ch[resolution-1];
                }else{
                    rate = SM.max_SD_rate_2chs[resolution-1];
                    rate_devider = SM.max_SD_rate_devider_2chs[resolution-1];
                }
            }
        }

        if (SM.rp_model == "Z20") {
            var max_possible_rate = 122.88e6;
            if (use_net) {
                if (channel_mode != 3) {
                    rate = SM.max_rate_1ch[resolution-1];
                    rate_devider = SM.max_rate_devider_1ch[resolution-1];
                }else{
                    rate = SM.max_rate_2chs[resolution-1];
                    rate_devider = SM.max_rate_devider_2chs[resolution-1];
                }
            }else {
                if (channel_mode != 3) {
                    rate = SM.max_SD_rate_1ch[resolution-1];
                    rate_devider = SM.max_SD_rate_devider_1ch[resolution-1];
                }else{
                    rate = SM.max_SD_rate_2chs[resolution-1];
                    rate_devider = SM.max_SD_rate_devider_2chs[resolution-1];
                }
            }
        }
        if (SM.ss_rate != -1) {
            var cur_rate = SM.ss_max_rate / SM.ss_rate;
            SM.ss_rate = rate / cur_rate;
            if (SM.ss_rate < 1)
                SM.ss_rate = 1;
        }
        SM.ss_max_rate = rate;
        SM.ss_max_rate_devider = rate_devider;
        rateFocusOutValue();
    };


}(window.SM = window.SM || {}, jQuery));