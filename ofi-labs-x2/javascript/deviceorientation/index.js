Ext.setup({

    onReady : function() {
        this.ui = new Ext.Panel({
            scroll: 'vertical',
            fullscreen: true,
            layout: 'fit',
            items: [{
                xtype: 'form',
                scroll: 'vertical',
                items: [{
                    xtype: 'fieldset',
                    title: 'Device Orientation Data',
                    items: [{
                        xtype: 'sliderfield',
                        label: 'Alpha',
                        id: 'Alpha',
                        disabled: true,
                        minValue: -180,
                        maxValue: 180
                    }, {
                        xtype: 'sliderfield',
                        label: 'Beta',
                        id: 'Beta',
                        disabled: true,
                        minValue: -90,
                        maxValue: 90
                    }, {
                        xtype: 'sliderfield',
                        label: 'Gamma',
                        id: 'Gamma',
                        disabled: true,
                        minValue: -180,
                        maxValue: 180
                    }]
                }]
            }]
        });

        this.ui.doComponentLayout();

        if (!('ondeviceorientation' in window)) {
            setTimeout(function() {
                Ext.Msg.alert('Not available', 'Your browser do not support device orientation.');
            }, 200);
        } else {
            var orientation;

            window.ondeviceorientation = function(e) {
                orientation = {
                    alpha: e.alpha,
                    beta: e.beta,
                    gamma: e.gamma
                };
            };

            window.setInterval(function() {
                Ext.getCmp('Alpha').setValue(Math.round(orientation.alpha), 200);
                Ext.getCmp('Beta').setValue(Math.round(orientation.beta), 200);
                Ext.getCmp('Gamma').setValue(Math.round(orientation.gamma), 200);
            }, 200);
        }
    }
});
