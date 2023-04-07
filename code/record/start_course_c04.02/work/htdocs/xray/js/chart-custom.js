(function(jQuery) {
    "use strict";
    jQuery(document).ready(function() {
        var rightSideBarMini = false;
        checkRightSideBar(rightSideBarMini);
        jQuery(document).on('click', '.right-sidebar-toggle', function() {
            if (rightSideBarMini) {
                rightSideBarMini = false;
            } else {
                rightSideBarMini = true;
            }
            checkRightSideBar(rightSideBarMini);
        })
    });

    function checkRightSideBar(rightSideBarMini) {
        if (rightSideBarMini) {
            rightSideBarShow();
        } else {
            rightSideBarHide()
        }
    }

    function rightSideBarShow() {
        jQuery('.right-sidebar-mini').addClass('right-sidebar')
    }

    function rightSideBarHide() {
        jQuery('.right-sidebar-mini').removeClass('right-sidebar')
    }

})(jQuery);


/*--------------Chart 1----------------*/

var options = {
    chart: {
        height: 80,
        type: 'area',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',

    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: 3,
        curve: 'smooth'
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            opacityFrom: 0.5,
            opacityTo: 0,
        }
    },

    series: [{
        name: 'series1',
        data: [60, 15, 50, 30, 70]
    }, ],
    colors: ['#089bab'],

    xaxis: {
        type: 'datetime',
        categories: ["2018-08-19T00:00:00", "2018-09-19T01:30:00", "2018-10-19T02:30:00", "2018-11-19T01:30:00", "2018-12-19T01:30:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};

if (jQuery('#chart-1').length) {
    var chart = new ApexCharts(
        document.querySelector("#chart-1"),
        options
    );
    chart.render();
}


/*--------------Chart 2----------------*/
var options = {
    chart: {
        height: 80,
        type: 'area',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',

    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: 3,
        curve: 'smooth'
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            opacityFrom: 0.5,
            opacityTo: 0,
        }
    },
    series: [{
        name: 'series1',
        data: [70, 40, 60, 30, 60]
    }, ],
    colors: ['#ffd400'],

    xaxis: {
        type: 'datetime',
        categories: ["2018-08-19T00:00:00", "2018-09-19T01:30:00", "2018-10-19T02:30:00", "2018-11-19T01:30:00", "2018-12-19T01:30:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};

if (jQuery('#chart-2').length) {
    var chart = new ApexCharts(
        document.querySelector("#chart-2"),
        options
    );

    chart.render();
}

/*--------------Chart 3----------------*/
var options = {
    chart: {
        height: 80,
        type: 'area',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',

    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: 3,
        curve: 'smooth'
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            opacityFrom: 0.5,
            opacityTo: 0,
        }
    },
    series: [{
        name: 'series1',
        data: [60, 40, 60, 40, 70]
    }, ],
    colors: ['#e64141'],

    xaxis: {
        type: 'datetime',
        categories: ["2018-08-19T00:00:00", "2018-09-19T01:30:00", "2018-10-19T02:30:00", "2018-11-19T01:30:00", "2018-12-19T01:30:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};
if (jQuery('#chart-3').length) {
    var chart = new ApexCharts(
        document.querySelector("#chart-3"),
        options
    );
    chart.render();
}

/*--------------Chart 4----------------*/
var options = {
    chart: {
        height: 80,
        type: 'area',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',

    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: 3,
        curve: 'smooth'
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            opacityFrom: 0.5,
            opacityTo: 0,
        }
    },
    series: [{
        name: 'series1',
        data: [75, 30, 60, 35, 60]
    }, ],
    colors: ['#e64141'],

    xaxis: {
        type: 'datetime',
        categories: ["2018-08-19T00:00:00", "2018-09-19T01:30:00", "2018-10-19T02:30:00", "2018-11-19T01:30:00", "2018-12-19T01:30:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};

if (jQuery('#chart-4').length) {
    var chart = new ApexCharts(
        document.querySelector("#chart-4"),
        options
    );

    chart.render();
}

/*--------------Chart 5----------------*/
var options = {
    chart: {
        height: 200,
        type: 'line',
        sparkline: {
            enabled: true
        },
        stacked: false,
    },
    stroke: {
        width: [2, 3],
        curve: 'smooth',
        dashArray: [0, 5]
    },
    plotOptions: {
        bar: {
            columnWidth: '50%'
        }
    },
    colors: ['#e64141', '#089bab'],
    series: [{
        name: 'Vine',
        type: 'area',
        data: [44, 55, 41, 67, 22, 43, 21, 41, 56, 27, 43]
    }, {
        name: 'Dribbble',
        type: 'line',
        data: [30, 25, 36, 30, 45, 35, 64, 52, 59, 36, 39]
    }],
    fill: {
        opacity: [0.2, 1],
        gradient: {
            inverseColors: false,
            shade: 'light',
            type: "vertical",
            opacityFrom: 1,
            opacityTo: 1,
            stops: [0, 100, 100, 100]
        }
    },
    labels: ['01/01/2003', '02/01/2003', '03/01/2003', '04/01/2003', '05/01/2003', '06/01/2003', '07/01/2003', '08/01/2003', '09/01/2003', '10/01/2003', '11/01/2003'],
    markers: {
        size: 0
    },
    xaxis: {
        type: 'datetime'
    },
    yaxis: {
        min: 0
    },
    tooltip: {
        shared: true,
        intersect: false,
        y: {
            formatter: function(y) {
                if (typeof y !== "undefined") {
                    return y.toFixed(0) + " views";
                }
                return y;

            }
        }
    },
    legend: {
        labels: {
            useSeriesColors: true
        },
        markers: {
            customHTML: [
                function() {
                    return ''
                },
                function() {
                    return ''
                },
                function() {
                    return ''
                }
            ]
        }
    }
};

if (jQuery('#chart-5').length) {
    var chart = new ApexCharts(
        document.querySelector("#chart-5"),
        options
    );

    chart.render();
}


/*--------------Chart 6----------------*/
var options = {
    chart: {
        height: 400,
        type: 'bar',
        sparkline: {
            show: false

        },
        toolbar: {
            show: false
        },
    },
    colors: ['#089bab', '#ffd400'],
    plotOptions: {
        bar: {
            horizontal: false,
            columnWidth: '30%',
            endingShape: 'rounded'
        },
    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        show: false,
        width: 5,
        colors: ['#ffffff'],
    },
    series: [{
        name: 'Male',
        enabled: 'true',
        data: [44, 90, 90, 60, 115]
    }, {
        name: 'Female',
        data: [35, 80, 100, 70, 95]
    }],


    fill: {
        opacity: 1

    },
    tooltip: {
        y: {
            formatter: function(val) {
                return "$ " + val + " thousands"
            }
        }
    }
};
options.colors = ['#089bab', '#FC9F5B'];
if (jQuery('#bar-chart-6').length) {
    var chart = new ApexCharts(
        document.querySelector("#bar-chart-6"),
        options
    );

    chart.render();
}
if (jQuery('#chart-6').length) {
    var chart = new ApexCharts(
        document.querySelector("#chart-6"),
        options
    );

    chart.render();
}


/*---------------- Chart - 7, Chart - 8 --------------------*/
var lastDate = 0;
var data = [];
var TICKINTERVAL = 86400000;
let XAXISRANGE = 777600000;

function getDayWiseTimeSeries(baseval, count, yrange) {
    var i = 0;
    while (i < count) {
        var x = baseval;
        var y = Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min;

        data.push({
            x,
            y
        });
        lastDate = baseval;
        baseval += TICKINTERVAL;
        i++;
    }
}

getDayWiseTimeSeries(new Date('11 Feb 2017 GMT').getTime(), 10, {
    min: 10,
    max: 90
});

function getNewSeries(baseval, yrange) {
    var newDate = baseval + TICKINTERVAL;
    lastDate = newDate;
    for (var i = 0; i < data.length - 10; i++) {
        data[i].x = newDate - XAXISRANGE - TICKINTERVAL;
        data[i].y = 0;
    }
    data.push({
        x: newDate,
        y: Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min
    })

}

function resetData() {
    // Alternatively, you can also reset the data at certain intervals to prevent creating a huge series
    data = data.slice(data.length - 10, data.length);
}


var options = {
    chart: {
        height: 150,
        type: 'area',
        animations: {
            enabled: true,
            easing: 'linear',
            dynamicAnimation: {
                speed: 1000
            }
        },
        toolbar: {
            show: false
        },
        sparkline: {
            enabled: true
        },
        group: 'sparklines',
    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        curve: 'straight',
        width: 3
    },
    series: [{
        data: data
    }],
    markers: {
        size: 4
    },
    xaxis: {
        type: 'datetime',
        range: XAXISRANGE,
    },
    yaxis: {
        max: 100
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            inverseColors: false,
            opacityFrom: 0.5,
            opacityTo: 0,
            stops: [0, 90, 100]
        },
    },
    legend: {
        show: false
    },
};
options.colors = ['#089bab'];

if (jQuery('#wave-chart-7').length) {
    options.markers.size = 0;
    options.chart.type = 'area';
    options.stroke.curve = "smooth";
    options.chart.height = 70;
    var wave_chart_7 = new ApexCharts(
        document.querySelector("#wave-chart-7"),
        options
    );
    wave_chart_7.render();
}
if (jQuery('#chart-7').length) {
    var chart_7 = new ApexCharts(
        document.querySelector("#chart-7"),
        options
    );
    chart_7.render();
}


options.colors = ['#FC9F5B'];
if (jQuery('#wave-chart-8').length) {
    options.markers.size = 0;
    options.chart.height = 70;
    options.stroke.curve = "smooth";
    options.chart.type = 'area';
    var wave_chart_8 = new ApexCharts(
        document.querySelector("#wave-chart-8"),
        options
    );
    wave_chart_8.render();
}
if (jQuery('#chart-8').length) {
    var chart_8 = new ApexCharts(
        document.querySelector("#chart-8"),
        options
    );
    chart_8.render();
}
options.colors = ['#00d0ff'];
if (jQuery('#wave-chart-9').length) {
    options.markers.size = 0;
    options.chart.height = 70;
    options.stroke.curve = "smooth";
    options.chart.type = 'area';
    var wave_chart_9 = new ApexCharts(
        document.querySelector("#wave-chart-9"),
        options
    );
    wave_chart_9.render();
}
if (jQuery('#chart-9').length) {
    var chart_9 = new ApexCharts(
        document.querySelector("#chart-9"),
        options
    );
    chart_9.render();
}
options.colors = ['#e64141'];
if (jQuery('#wave-chart-10').length) {
    options.markers.size = 0;
    options.chart.height = 70;
    options.stroke.curve = "smooth";
    options.chart.type = 'area';
    var wave_chart_10 = new ApexCharts(
        document.querySelector("#wave-chart-10"),
        options
    );
    wave_chart_10.render();
}
if (jQuery('#chart-10').length) {
    var chart_10 = new ApexCharts(
        document.querySelector("#chart-10"),
        options
    );
    chart_10.render();
}
if (jQuery('#wave-chart-7').length || jQuery('#wave-chart-8').length || jQuery('#wave-chart-9').length || jQuery('#wave-chart-10').length) {
    window.setInterval(function() {
        getNewSeries(lastDate, {
            min: 10,
            max: 90
        });
        if (jQuery('#wave-chart-7').length) {
            wave_chart_7.updateSeries([{
                data: data
            }]);
        }
        if (jQuery('#wave-chart-8').length) {
            wave_chart_8.updateSeries([{
                data: data
            }])
        }
        if (jQuery('#wave-chart-9').length) {
            wave_chart_9.updateSeries([{
                data: data
            }])
        }
        if (jQuery('#wave-chart-10').length) {
            wave_chart_10.updateSeries([{
                data: data
            }])
        }
    }, 1000);
}

/*--------------- Chart-9 --------------*/
var options = {
    chart: {
        height: 112,
        type: 'area',
        animations: {
            enabled: true,
            easing: 'linear',
            dynamicAnimation: {
                speed: 1000
            }
        },
        toolbar: {
            show: false
        },
        sparkline: {
            enabled: true
        },
        group: 'sparklines',
    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        curve: 'smooth',
        width: 3
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            opacityFrom: 0.5,
            opacityTo: 0,
        }
    },
    series: [{
        data: data
    }],
    markers: {
        size: 0
    },
    xaxis: {
        type: 'datetime',
        range: XAXISRANGE,
    },
    yaxis: {
        max: 100
    },
    legend: {
        show: false
    },
};
options.colors = ['#089bab'];

if (jQuery('#chart-9').length) {
    var chart9 = new ApexCharts(
        document.querySelector("#chart-9"),
        options
    );
    chart9.render();
    window.setInterval(function() {
        getNewSeries(lastDate, {
            min: 10,
            max: 90
        });
        if (jQuery('#chart-9').length) {
            chart9.updateSeries([{
                data: data
            }]);
        }
    }, 1000);
}

/*----------------- Chart-10 ----------------*/

function generateData(baseval, count, yrange) {
    var i = 0;
    var series = [];
    while (i < count) {
        var x = Math.floor(Math.random() * (750 - 1 + 1)) + 1;;
        var y = Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min;
        var z = Math.floor(Math.random() * (75 - 15 + 1)) + 15;

        series.push([x, y, z]);
        baseval += 86400000;
        i++;
    }
    return series;
}

var options = {
    chart: {
        height: 440,
        type: 'bubble',
        stacked: false,
        toolbar: {
            show: false
        },
        animations: {
            enabled: true,
            easing: 'linear',
            dynamicAnimation: {
                speed: 1000
            }
        },
        sparkline: {
            enabled: true
        },
        group: 'sparklines',
    },
    dataLabels: {
        enabled: false
    },
    series: [{
        name: 'Bubble1',
        data: generateData(new Date('11 Feb 2017 GMT').getTime(), 10, {
            min: 10,
            max: 60
        })
    }, {
        name: 'Bubble2',
        data: generateData(new Date('11 Feb 2017 GMT').getTime(), 10, {
            min: 10,
            max: 60
        })
    }, {
        name: 'Bubble3',
        data: generateData(new Date('11 Feb 2017 GMT').getTime(), 10, {
            min: 10,
            max: 60
        })
    }, {
        name: 'Bubble4',
        data: generateData(new Date('11 Feb 2017 GMT').getTime(), 10, {
            min: 10,
            max: 60
        })
    }],
    fill: {
        opacity: 0.8
    },
    title: {
        show: false
    },
    xaxis: {
        tickAmount: 8,
        type: 'category',
        labels: {
            show: false
        }
    },
    yaxis: {
        max: 70,
        labels: {
            show: false
        }
    },
    legend: {
        show: false
    }
};

if (jQuery('#chart-10').length) {
    var chart_2 = new ApexCharts(
        document.querySelector("#chart-10"),
        options
    );

    chart_2.render();
}


/*----------------- Chart-11 ----------------*/
var chart11 = {
    chart: {
        height: 407,
        type: 'line',
        zoom: {
            enabled: false
        },
        toolbar: {
            show: false
        },
    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: [5, 7, 5],
        curve: 'straight',
        dashArray: [0, 8, 5]
    },
    series: [{
        name: "Session Duration",
        data: [45, 52, 38, 24, 33, 26, 21, 20, 6, 8, 15, 10]
    }, {
        name: "Page Views",
        data: [35, 41, 62, 42, 13, 18, 29, 37, 36, 51, 32, 35]
    }, {
        name: 'Total Visits',
        data: [87, 57, 74, 99, 75, 38, 62, 47, 82, 56, 45, 47]
    }],
    legend: {
        show: false
    },
    markers: {
        size: 0,

        hover: {
            sizeOffset: 6
        }
    },
    xaxis: {
        categories: ['01 Jan', '02 Jan', '03 Jan', '04 Jan', '05 Jan', '06 Jan', '07 Jan', '08 Jan', '09 Jan',
            '10 Jan', '11 Jan', '12 Jan'
        ]
    },
    yaxis: {
        labels: {
            show: false
        }
    },
    tooltip: {
        y: [{
            title: {
                formatter: function(val) {
                    return val + " (mins)"
                }
            }
        }, {
            title: {
                formatter: function(val) {
                    return val + " per session"
                }
            }
        }, {
            title: {
                formatter: function(val) {
                    return val;
                }
            }
        }]
    },
    grid: {
        borderColor: '#f1f1f1',
    }
}
if (jQuery('#chart-11').length) {
    var chart_3 = new ApexCharts(
        document.querySelector("#chart-11"),
        chart11
    );
    chart_3.render();
}


/*--------------Chart 12----------------*/
var options12 = {
    chart: {
        height: 140,
        type: 'area',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',

    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: 5,
        curve: 'smooth'
    },
    fill: {
        type: 'gradient',
        gradient: {
            shadeIntensity: 1,
            opacityFrom: 0.5,
            opacityTo: 0,
        }
    },
    series: [{
        name: 'series1',
        data: [70, 40, 60, 30, 60]
    }, ],
    colors: ['#ffd400'],

    xaxis: {
        type: 'datetime',
        categories: ["2018-08-19T00:00:00", "2018-09-19T01:30:00", "2018-10-19T02:30:00", "2018-11-19T01:30:00", "2018-12-19T01:30:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};
if (jQuery('#chart-12').length) {
    var chart12 = new ApexCharts(
        document.querySelector("#chart-12"),
        options12
    );

    chart12.render();
}

var barOptions12 = {
    chart: {
        height: 150,
        type: 'bar',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',
    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        curve: 'smooth',
        width: 15
    },
    series: [{
        name: 'series1',
        data: [70, 40, 60, 30, 60, ]
    }, ],
    colors: ['#e3e9ef'],
    xaxis: {
        type: 'datetime',
        categories: ["2019-08-19T00:00:00", "2019-09-19T00:00:00", "2019-10-19T00:00:00", "2019-11-19T00:00:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};

if (jQuery('#bar-chart-12').length) {
    var chart12 = new ApexCharts(
        document.querySelector("#bar-chart-12"),
        barOptions12
    );

    chart12.render();
    window.setInterval(function() {
        getNewSeries(lastDate, {
            min: 10,
            max: 90
        });
        if (jQuery('#bar-chart-12').length) {
            chart12.updateSeries([{
                data: data
            }]);
        }
    }, 1000);
}



/*--------------Chart 13----------------*/
var option13 = {
    chart: {
        height: 420,
        type: 'radialBar',
        stroke: {
            show: true,
            curve: 'smooth',
            lineCap: 'butt',
            colors: undefined,
            width: 3,
            dashArray: 0,
        },
    },
    plotOptions: {
        radialBar: {
            hollow: {
                margin: 10,
                size: '30%',
                background: 'transparent',
                image: undefined,
                imageWidth: 64,
                imageHeight: 64,
            },
            dataLabels: {
                name: {
                    fontSize: '22px',
                },
                value: {
                    fontSize: '16px',
                },
                total: {
                    show: true,
                    label: 'Total',
                    formatter: function(w) {
                        // By default this function returns the average of all series. The below is just an example to show the use of custom formatter function
                        return 249
                    }
                }
            }
        }
    },
    fill: {
        type: 'gradient',
    },
    colors: ['#089bab', '#e64141', '#ffd400'],
    series: [44, 55, 67],
    stroke: {
        lineCap: 'round'
    },
    labels: [' Mobile', 'Desktop', 'Tablet', 'Watch'],
};

if (jQuery('#chart-13').length) {
    var chart13 = new ApexCharts(
        document.querySelector("#chart-13"),
        option13
    );
    chart13.render();

}

/*--------------Chart 13----------------*/

var option14 = {
    chart: {
        height: 350,
        type: 'radialBar',
    },
    plotOptions: {
        hollow: {
            margin: 10,
            size: '30%',
            background: 'transparent',
            image: undefined,
            imageWidth: 64,
            imageHeight: 64,
        },
        radialBar: {
            startAngle: -135,
            endAngle: 135,
            dataLabels: {
                name: {
                    fontSize: '16px',
                    color: undefined,
                    offsetY: 120
                },
                value: {
                    offsetY: 76,
                    fontSize: '22px',
                    color: undefined,
                    formatter: function(val) {
                        return val + "%";
                    }
                }
            }
        }
    },
    fill: {},
    stroke: {
        dashArray: 5,
    },
    series: [67],
    labels: ['User traffic'],
    colors: ['#089bab'],

};

if (jQuery('#chart-14').length) {
    var chart14 = new ApexCharts(
        document.querySelector("#chart-14"),
        option14
    );

    chart14.render();
}



/*--------------Chart 15----------------*/
var option15 = {
    chart: {
        height: 310,
        type: 'line',
        stacked: false,
        toolbar: {
            show: false
        }
    },
    dataLabels: {
        enabled: false
    },
    colors: ['#e64141', '#089bab', '#e64141'],
    series: [{
        name: 'Income',
        type: 'column',
        data: [14, 2, 25, 15, 25, 28, 38, 46]
    }, {
        name: 'Cashflow',
        type: 'column',
        data: [11, 3, 31, 4, 41, 49, 65, 85]
    }, {
        name: 'Revenue',
        type: 'line',
        data: [20, 29, 37, 36, 44, 45, 50, 58]
    }],
    stroke: {
        width: [1, 1, 4]
    },
    xaxis: {
        categories: [2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016],
        labels: {
            show: false
        }
    },
    yaxis: {
        show: false
    },
    tooltip: {
        theme: 'dark',
        fixed: {
            enabled: true,
            position: 'topLeft', // topRight, topLeft, bottomRight, bottomLeft
            offsetY: 30,
            offsetX: 60
        },
    },
    legend: {
        show: false,
    }
};

if (jQuery('#chart-15').length) {
    var chart15 = new ApexCharts(
        document.querySelector("#chart-15"),
        option15
    );

    chart15.render();
}

/*--------------Chart 16----------------*/

var option16 = {
    chart: {
        height: 350,
        type: 'area',
        sparkline: {
            enabled: true
        },
        group: 'sparklines',

    },
    dataLabels: {
        enabled: false
    },
    stroke: {
        curve: 'smooth'
    },
    series: [{
        name: 'series1',
        data: [3, 30, 15, 25, 35]
    }, ],
    colors: ['#6f32ff'],

    xaxis: {
        type: 'datetime',
        categories: ["2018-08-19T00:00:00", "2018-09-19T01:30:00", "2018-10-19T02:30:00", "2018-11-19T01:30:00", "2018-12-19T01:30:00"],
    },
    tooltip: {
        x: {
            format: 'dd/MM/yy HH:mm'
        },
    }
};

if (jQuery('#chart-16').length) {
    var chart16 = new ApexCharts(
        document.querySelector("#chart-16"),
        option16
    );

    chart16.render();
}

/*--------------Chart 17----------------*/

var option17 = {
    chart: {
        height: 268,
        type: 'area',
        zoom: {
            enabled: false
        },
        toolbar: {
            show: false
        },
        sparkline: {
            enabled: true
        },
        group: 'sparklines',
    },
    colors: ['#089bab', '#e64141', '#ffd400'],
    dataLabels: {
        enabled: false
    },
    stroke: {
        width: [3, 2, 3],
        curve: 'straight',
    },
    series: [{
        name: "Session Duration",
        data: [45, 52, 38, 24, 33, 26, 21, 20, 6, 8, 15, 10]
    }, {
        name: "Page Views",
        data: [35, 41, 62, 42, 13, 18, 29, 37, 36, 51, 32, 35]
    }, {
        name: 'Total Visits',
        data: [87, 57, 74, 30, 75, 38, 62, 47, 82, 56, 45, 47]
    }],
    legend: {
        show: false
    },
    markers: {
        size: 0,

        hover: {
            sizeOffset: 6
        }
    },
    xaxis: {
        categories: ['01 Jan', '02 Jan', '03 Jan', '04 Jan', '05 Jan', '06 Jan', '07 Jan', '08 Jan', '09 Jan',
            '10 Jan', '11 Jan', '12 Jan'
        ],
        labels: {
            show: false
        }
    },
    yaxis: {
        labels: {
            show: false
        }
    },
    tooltip: {
        y: [{
            title: {
                formatter: function(val) {
                    return val + " (mins)"
                }
            }
        }, {
            title: {
                formatter: function(val) {
                    return val + " per session"
                }
            }
        }, {
            title: {
                formatter: function(val) {
                    return val;
                }
            }
        }]
    },
    grid: {
        borderColor: '#f1f1f1',
    }
}

if (jQuery('#Dash1MultiBarChart').length) {
    var chart17 = new ApexCharts(
        document.querySelector("#Dash1MultiBarChart"),
        option17
    );

    chart17.render();
}

/*--------------Chart 18----------------*/


var option18 = {
    chart: {
        height: 235,
        type: 'donut',
    },
    plotOptions: {
        pie: {
            donut: {
                size: '55%'
            }
        }
    },
    series: [44, 55, 41],
    colors: ['#e64141', '#e64141', '#089bab'],
    labels: ['Server1', 'Server2', 'Server3'],
    legend: {
        position: 'bottom'
    }
};

if (jQuery('#Dash1DonetChart').length) {
    var chart18 = new ApexCharts(
        document.querySelector("#Dash1DonetChart"),
        option18
    );

    chart18.render();
}


var option19 = {
    chart: {
        height: 308,
        type: 'line',
        toolbar: {
            show: false
        }
    },
    series: [{
        name: 'Website Blog',
        type: 'column',
        data: [440, 505, 414, 671, 227, 413, 201, 352, 752, 320, 257, 160]
    }, {
        name: 'Social Media',
        type: 'line',
        data: [23, 42, 35, 27, 43, 22, 17, 31, 22, 22, 12, 16]
    }],
    stroke: {
        width: [0, 4]
    },
    title: {
        show: false
    },
    legend: {
        show: false
    },
    // labels: ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"],
    labels: ['01 Jan 2001', '02 Jan 2001', '03 Jan 2001', '04 Jan 2001', '05 Jan 2001', '06 Jan 2001', '07 Jan 2001', '08 Jan 2001', '09 Jan 2001', '10 Jan 2001', '11 Jan 2001', '12 Jan 2001'],
    colors: ['#089bab', '#e64141'],
    xaxis: {
        labels: {
            show: false,
        },
        type: 'datetime'
    },
    yaxis: [{
        labels: {
            show: false,
        }
    }, {
        labels: {
            show: false,
        }
    }],
};

if (jQuery('#chart-19').length) {

    var chart19 = new ApexCharts(
        document.querySelector("#chart-19"),
        option19
    );

    chart19.render();
}

var option20 = {
    chart: {
        height: 387,
        type: 'line',
        stacked: false,
        toolbar: {
            show: false
        },
        sparkline: {
            enabled: true
        },
        group: 'sparklines',
    },
    stroke: {
        width: [2, 3],
        curve: 'smooth'
    },
    plotOptions: {
        bar: {
            columnWidth: '20%'
        }
    },
    colors: ['#e64141', '#089bab'],
    series: [{
        name: 'Vine',
        type: 'area',
        data: [44, 55, 41, 67, 22, 43, 21, 41, 56, 27, 43]
    }, {
        name: 'Dribbble',
        type: 'line',
        data: [30, 25, 36, 30, 45, 35, 64, 52, 59, 36, 39]
    }],
    fill: {
        opacity: [0.25, 1],
        gradient: {
            inverseColors: false,
            shade: 'light',
            type: "vertical",
            opacityFrom: 0.85,
            opacityTo: 0.55,
            stops: [0, 100, 100, 100]
        }
    },
    labels: ['01/01/2003', '02/01/2003', '03/01/2003', '04/01/2003', '05/01/2003', '06/01/2003', '07/01/2003', '08/01/2003', '09/01/2003', '10/01/2003', '11/01/2003'],

    xaxis: {
        type: 'datetime',
        labels: {
            show: false
        }
    },
    yaxis: {
        min: 0,
        labels: {
            show: false
        }
    },
    tooltip: {
        shared: true,
        intersect: false,
        y: {
            formatter: function(y) {
                if (typeof y !== "undefined") {
                    return y.toFixed(0) + " views";
                }
                return y;

            }
        }
    },
    legend: {
        show: false
    }
};


if (jQuery('#chart-20').length) {

    var chart20 = new ApexCharts(
        document.querySelector("#chart-20"),
        option20
    );

    chart20.render();
}



/**
 * ---------------------------------------
 * This demo was created using amCharts 4.
 *
 * For more information visit:
 * https://www.amcharts.com/
 *
 * Documentation is available at:
 * https://www.amcharts.com/docs/v4/
 * ---------------------------------------
 */
if (jQuery('#amChartWorldMap').length) {
    // Themes begin
    am4core.useTheme(am4themes_animated);
    // Themes end

    // Create map instance
    var chart = am4core.create("amChartWorldMap", am4maps.MapChart);

    // Set map definition
    chart.geodata = am4geodata_worldLow;

    // Set projection
    chart.projection = new am4maps.projections.Miller();

    // Series for World map
    var worldSeries = chart.series.push(new am4maps.MapPolygonSeries());
    worldSeries.exclude = ["AQ"];
    worldSeries.useGeodata = true;

    var polygonTemplate = worldSeries.mapPolygons.template;
    polygonTemplate.tooltipText = "{name}";
    polygonTemplate.fill = chart.colors.getIndex(0);
    polygonTemplate.nonScalingStroke = true;

    // Hover state
    var hs = polygonTemplate.states.create("hover");
    hs.properties.fill = am4core.color("#367B25");

    // Series for United States map
    var usaSeries = chart.series.push(new am4maps.MapPolygonSeries());
    usaSeries.geodata = am4geodata_usaLow;

    var usPolygonTemplate = usaSeries.mapPolygons.template;
    usPolygonTemplate.tooltipText = "{name}";
    usPolygonTemplate.fill = chart.colors.getIndex(1);
    usPolygonTemplate.nonScalingStroke = true;

    // Hover state
    var hs = usPolygonTemplate.states.create("hover");
    hs.properties.fill = am4core.color("#367B25");
}
// Web Analytics Chart
if (jQuery('#chartdiv').length) {
    jQuery(document).ready(function() {
        am4core.ready(function() {
            // Themes begin
            am4core.useTheme(am4themes_animated);
            // Themes end

            // Define country data
            var countries = {
                "AD": {
                    "country": "Andorra",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["andorraLow", "andorraHigh"]
                },
                "AE": {
                    "country": "United Arab Emirates",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["uaeLow", "uaeHigh"]
                },
                "AF": {
                    "country": "Afghanistan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "AG": {
                    "country": "Antigua and Barbuda",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["antiguaBarbudaLow", "antiguaBarbudaHigh"]
                },
                "AI": {
                    "country": "Anguilla",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["anguillaLow", "anguillaHigh"]
                },
                "AL": {
                    "country": "Albania",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["albaniaLow", "albaniaHigh"]
                },
                "AM": {
                    "country": "Armenia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["armeniaLow", "armeniaHigh"]
                },
                "AO": {
                    "country": "Angola",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["angolaLow", "angolaHigh"]
                },
                "AQ": {
                    "country": "Antarctica",
                    "continent_code": "AN",
                    "continent": "Antarctica",
                    "maps": []
                },
                "AR": {
                    "country": "Argentina",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["argentinaLow", "argentinaHigh"]
                },
                "AS": {
                    "country": "American Samoa",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": ["americanSamoaLow", "americanSamoaHigh"]
                },
                "AT": {
                    "country": "Austria",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["austriaLow", "austriaHigh"]
                },
                "AU": {
                    "country": "Australia",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": ["australiaLow", "australiaHigh"]
                },
                "AW": {
                    "country": "Aruba",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["arubaLow", "arubaHigh"]
                },
                "AX": {
                    "country": "Aland Islands",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "AZ": {
                    "country": "Azerbaijan",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["azerbaijanLow", "azerbaijanHigh"]
                },
                "BA": {
                    "country": "Bosnia and Herzegovina",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["bosniaHerzegovinaLow", "bosniaHerzegovinaHigh", "bosniaHerzegovinaCantonsLow", "bosniaHerzegovinaCantonsHigh"]
                },
                "BB": {
                    "country": "Barbados",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["barbadosLow", "barbadosHigh"]
                },
                "BD": {
                    "country": "Bangladesh",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["bangladeshLow", "bangladeshHigh"]
                },
                "BE": {
                    "country": "Belgium",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["belgiumLow", "belgiumHigh"]
                },
                "BF": {
                    "country": "Burkina Faso",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["burkinaFasoLow", "burkinaFasoHigh"]
                },
                "BG": {
                    "country": "Bulgaria",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["bulgariaLow", "bulgariaHigh"]
                },
                "BH": {
                    "country": "Bahrain",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["bahrainLow", "bahrainHigh"]
                },
                "BI": {
                    "country": "Burundi",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["burundiLow", "burundiHigh"]
                },
                "BJ": {
                    "country": "Benin",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["beninLow", "beninHigh"]
                },
                "BL": {
                    "country": "Saint Barthelemy",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "BM": {
                    "country": "Bermuda",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["bermudaLow", "bermudaHigh"]
                },
                "BN": {
                    "country": "Brunei Darussalam",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["bruneiDarussalamLow", "bruneiDarussalamHigh"]
                },
                "BO": {
                    "country": "Bolivia, Plurinational State of",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["boliviaLow", "boliviaHigh"]
                },
                "BQ": {
                    "country": "Bonaire, Sint Eustatius and Saba",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["bonaireSintEustatiusSabaLow", "bonaireSintEustatiusSabaHigh"]
                },
                "BR": {
                    "country": "Brazil",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["brazilLow", "brazilHigh"]
                },
                "BS": {
                    "country": "Bahamas",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "BT": {
                    "country": "Bhutan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["bhutanLow", "bhutanHigh"]
                },
                "BV": {
                    "country": "Bouvet Island",
                    "continent_code": "AN",
                    "continent": "Antarctica",
                    "maps": []
                },
                "BW": {
                    "country": "Botswana",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["botswanaLow", "botswanaHigh"]
                },
                "BY": {
                    "country": "Belarus",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["belarusLow", "belarusHigh"]
                },
                "BZ": {
                    "country": "Belize",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["belizeLow", "belizeHigh"]
                },
                "CA": {
                    "country": "Canada",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["canadaLow", "canadaHigh"]
                },
                "CC": {
                    "country": "Cocos (Keeling) Islands",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "CD": {
                    "country": "Congo, the Democratic Republic of the",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["congoDRLow", "congoDRHigh"]
                },
                "CF": {
                    "country": "Central African Republic",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["centralAfricanRepublicLow", "centralAfricanRepublicHigh"]
                },
                "CG": {
                    "country": "Congo",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["congoLow", "congoHigh"]
                },
                "CH": {
                    "country": "Switzerland",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["switzerlandLow", "switzerlandHigh"]
                },
                "CI": {
                    "country": "Cote d'Ivoire",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "CK": {
                    "country": "Cook Islands",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "CL": {
                    "country": "Chile",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["chileLow", "chileHigh"]
                },
                "CM": {
                    "country": "Cameroon",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["cameroonLow", "cameroonHigh"]
                },
                "CN": {
                    "country": "China",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["chinaLow", "chinaHigh"]
                },
                "CO": {
                    "country": "Colombia",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["colombiaLow", "colombiaHigh", "colombiaMuniLow", "colombiaMuniHigh"]
                },
                "CR": {
                    "country": "Costa Rica",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["costaRicaLow", "costaRicaHigh"]
                },
                "CU": {
                    "country": "Cuba",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "CV": {
                    "country": "Cape Verde",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["capeVerdeLow", "capeVerdeHigh"]
                },
                "CW": {
                    "country": "Curacao",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["curacaoLow", "curacaoHigh"]
                },
                "CX": {
                    "country": "Christmas Island",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "CY": {
                    "country": "Cyprus",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["cyprusLow", "cyprusHigh", "cyprusNorthCyprusLow", "cyprusNorthCyprusHigh"]
                },
                "CZ": {
                    "country": "Czech Republic",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["czechiaLow", "czechiaHigh"]
                },
                "DE": {
                    "country": "Germany",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["germanyLow", "germanyHigh"]
                },
                "DJ": {
                    "country": "Djibouti",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["djiboutiLow", "djiboutiHigh"]
                },
                "DK": {
                    "country": "Denmark",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["denmarkLow", "denmarkHigh"]
                },
                "DM": {
                    "country": "Dominica",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["dominicaLow", "dominicaHigh"]
                },
                "DO": {
                    "country": "Dominican Republic",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["dominicanRepublicLow", "dominicanRepublicHigh", "dominicanRepublicMuniLow", "dominicanRepublicMuniHigh"]
                },
                "DZ": {
                    "country": "Algeria",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["algeriaLow", "algeriaHigh"]
                },
                "EC": {
                    "country": "Ecuador",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["ecuadorLow", "ecuadorHigh"]
                },
                "EE": {
                    "country": "Estonia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["estoniaLow", "estoniaHigh"]
                },
                "EG": {
                    "country": "Egypt",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["egyptLow", "egyptHigh"]
                },
                "EH": {
                    "country": "Western Sahara",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "ER": {
                    "country": "Eritrea",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "ES": {
                    "country": "Spain",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["spainLow", "spainHigh", "spainProvincesLow", "spainProvincesHigh"]
                },
                "ET": {
                    "country": "Ethiopia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "FI": {
                    "country": "Finland",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["finlandLow", "finlandHigh"]
                },
                "FJ": {
                    "country": "Fiji",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": ["fijiEastLow", "fijiEastHigh", "fijiWestLow", "fijiWestHigh"]
                },
                "FK": {
                    "country": "Falkland Islands (Malvinas)",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": []
                },
                "FM": {
                    "country": "Micronesia, Federated States of",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "FO": {
                    "country": "Faroe Islands",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["faroeIslandsLow", "faroeIslandsHigh"]
                },
                "FR": {
                    "country": "France",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["franceLow", "franceHigh", "franceDepartmentsLow", "franceDepartmentsHigh"]
                },
                "GA": {
                    "country": "Gabon",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["gabonLow", "gabonHigh"]
                },
                "GB": {
                    "country": "United Kingdom",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["ukLow", "ukHigh", "ukCountiesLow", "ukCountiesHigh"]
                },
                "GB-CHA": {
                    "country": "Channel Islands",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["channelIslandsLow", "channelIslandsHigh"]
                },
                "GD": {
                    "country": "Grenada",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "GE": {
                    "country": "Georgia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["georgiaLow", "georgiaHigh", "georgiaSouthOssetiaLow", "georgiaSouthOssetiaHigh"]
                },
                "GF": {
                    "country": "French Guiana",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["frenchGuianaLow", "frenchGuianaHigh"]
                },
                "GG": {
                    "country": "Guernsey",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "GH": {
                    "country": "Ghana",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "GI": {
                    "country": "Gibraltar",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "GL": {
                    "country": "Greenland",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["greenlandLow", "greenlandHigh"]
                },
                "GM": {
                    "country": "Gambia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "GN": {
                    "country": "Guinea",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["guineaLow", "guineaHigh"]
                },
                "GP": {
                    "country": "Guadeloupe",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "GQ": {
                    "country": "Equatorial Guinea",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["equatorialGuineaLow", "equatorialGuineaHigh"]
                },
                "GR": {
                    "country": "Greece",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["greeceLow", "greeceHigh"]
                },
                "GS": {
                    "country": "South Georgia and the South Sandwich Islands",
                    "continent_code": "AN",
                    "continent": "Antarctica",
                    "maps": []
                },
                "GT": {
                    "country": "Guatemala",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "GU": {
                    "country": "Guam",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "GW": {
                    "country": "Guinea-Bissau",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "GY": {
                    "country": "Guyana",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": []
                },
                "HK": {
                    "country": "Hong Kong",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["hongKongLow", "hongKongHigh"]
                },
                "HM": {
                    "country": "Heard Island and McDonald Islands",
                    "continent_code": "AN",
                    "continent": "Antarctica",
                    "maps": []
                },
                "HN": {
                    "country": "Honduras",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["hondurasLow", "hondurasHigh"]
                },
                "HR": {
                    "country": "Croatia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["croatiaLow", "croatiaHigh"]
                },
                "HT": {
                    "country": "Haiti",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "HU": {
                    "country": "Hungary",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["hungaryLow", "hungaryHigh"]
                },
                "ID": {
                    "country": "Indonesia",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["indonesiaLow", "indonesiaHigh"]
                },
                "IE": {
                    "country": "Ireland",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["irelandLow", "irelandHigh"]
                },
                "IL": {
                    "country": "Israel",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["israelLow", "israelHigh", "israelPalestineLow", "israelPalestineHigh"]
                },
                "IM": {
                    "country": "Isle of Man",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "IN": {
                    "country": "India",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["indiaLow", "indiaHigh"]
                },
                "IO": {
                    "country": "British Indian Ocean Territory",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "IQ": {
                    "country": "Iraq",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "IR": {
                    "country": "Iran, Islamic Republic of",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "IS": {
                    "country": "Iceland",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["icelandLow", "icelandHigh"]
                },
                "IT": {
                    "country": "Italy",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["italyLow", "italyHigh"]
                },
                "JE": {
                    "country": "Jersey",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "JM": {
                    "country": "Jamaica",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "JO": {
                    "country": "Jordan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "JP": {
                    "country": "Japan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["japanLow", "japanHigh"]
                },
                "KE": {
                    "country": "Kenya",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["kenyaLow", "kenyaHigh"]
                },
                "KG": {
                    "country": "Kyrgyzstan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["kyrgyzstanLow", "kyrgyzstanHigh"]
                },
                "KH": {
                    "country": "Cambodia",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["cambodiaLow", "cambodiaHigh"]
                },
                "KI": {
                    "country": "Kiribati",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "KM": {
                    "country": "Comoros",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "KN": {
                    "country": "Saint Kitts and Nevis",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "KP": {
                    "country": "Korea, Democratic People's Republic of",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["northKoreaLow", "northKoreaHigh"]
                },
                "KR": {
                    "country": "Korea, Republic of",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["southKoreaLow", "southKoreaHigh"]
                },
                "KT": {
                    "country": "Saint Kitts and Nevis",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["stKittsNevisLow", "stKittsNevisHigh"]
                },
                "KW": {
                    "country": "Kuwait",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "KY": {
                    "country": "Cayman Islands",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "KZ": {
                    "country": "Kazakhstan",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["kazakhstanLow", "kazakhstanHigh"]
                },
                "LA": {
                    "country": "Lao People's Democratic Republic",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "LB": {
                    "country": "Lebanon",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "LC": {
                    "country": "Saint Lucia",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["saintLuciaLow", "saintLuciaHigh"]
                },
                "LI": {
                    "country": "Liechtenstein",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["liechtensteinLow", "liechtensteinHigh"]
                },
                "LK": {
                    "country": "Sri Lanka",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["sriLankaLow", "sriLankaHigh"]
                },
                "LR": {
                    "country": "Liberia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "LS": {
                    "country": "Lesotho",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "LT": {
                    "country": "Lithuania",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["lithuaniaLow", "lithuaniaHigh"]
                },
                "LU": {
                    "country": "Luxembourg",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "LV": {
                    "country": "Latvia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["latviaLow", "latviaHigh"]
                },
                "LY": {
                    "country": "Libya",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "MA": {
                    "country": "Morocco",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["moroccoLow", "moroccoHigh"]
                },
                "MC": {
                    "country": "Monaco",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "MD": {
                    "country": "Moldova, Republic of",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["moldovaLow", "moldovaHigh"]
                },
                "ME": {
                    "country": "Montenegro",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "MF": {
                    "country": "Saint Martin (French Part)",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "MG": {
                    "country": "Madagascar",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "MH": {
                    "country": "Marshall Islands",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "MK": {
                    "country": "North Macedonia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": []
                },
                "ML": {
                    "country": "Mali",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["maliLow", "maliHigh"]
                },
                "MM": {
                    "country": "Myanmar",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "MN": {
                    "country": "Mongolia",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["mongoliaLow", "mongoliaHigh"]
                },
                "MO": {
                    "country": "Macao",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "MP": {
                    "country": "Northern Mariana Islands",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "MQ": {
                    "country": "Martinique",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "MR": {
                    "country": "Mauritania",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "MS": {
                    "country": "Montserrat",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "MT": {
                    "country": "Malta",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["maltaLow", "maltaHigh"]
                },
                "MU": {
                    "country": "Mauritius",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "MV": {
                    "country": "Maldives",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["maldivesLow", "maldivesHigh", "maldivesIslandsLow", "maldivesIslandsHigh"]
                },
                "MW": {
                    "country": "Malawi",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "MX": {
                    "country": "Mexico",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["mexicoLow", "mexicoHigh"]
                },
                "MY": {
                    "country": "Malaysia",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["malaysiaLow", "malaysiaHigh"]
                },
                "MZ": {
                    "country": "Mozambique",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "NA": {
                    "country": "Namibia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["namibiaLow", "namibiaHigh"]
                },
                "NC": {
                    "country": "New Caledonia",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "NE": {
                    "country": "Niger",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "NF": {
                    "country": "Norfolk Island",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "NG": {
                    "country": "Nigeria",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["nigeriaLow", "nigeriaHigh"]
                },
                "NI": {
                    "country": "Nicaragua",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["nicaraguaLow", "nicaraguaHigh"]
                },
                "NL": {
                    "country": "Netherlands",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["netherlandsLow", "netherlandsHigh"]
                },
                "NO": {
                    "country": "Norway",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["norwayLow", "norwayHigh"]
                },
                "NP": {
                    "country": "Nepal",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["nepalLow", "nepalHigh"]
                },
                "NR": {
                    "country": "Nauru",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "NU": {
                    "country": "Niue",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "NZ": {
                    "country": "New Zealand",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": ["newZealandLow", "newZealandHigh"]
                },
                "OM": {
                    "country": "Oman",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["omanLow", "omanHigh"]
                },
                "PA": {
                    "country": "Panama",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["panamaLow", "panamaHigh"]
                },
                "PE": {
                    "country": "Peru",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["peruLow", "peruHigh"]
                },
                "PF": {
                    "country": "French Polynesia",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "PG": {
                    "country": "Papua New Guinea",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "PH": {
                    "country": "Philippines",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["philippinesLow", "philippinesHigh"]
                },
                "PK": {
                    "country": "Pakistan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["pakistanLow", "pakistanHigh"]
                },
                "PL": {
                    "country": "Poland",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["polandLow", "polandHigh"]
                },
                "PM": {
                    "country": "Saint Pierre and Miquelon",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["stPierreMiquelonLow", "stPierreMiquelonHigh"]
                },
                "PN": {
                    "country": "Pitcairn",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "PR": {
                    "country": "Puerto Rico",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["puertoRicoLow", "puertoRicoHigh"]
                },
                "PS": {
                    "country": "Palestinian, State of",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["palestineLow", "palestineHigh"]
                },
                "PT": {
                    "country": "Portugal",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["portugalLow", "portugalHigh", "portugalRegionsLow", "portugalRegionsHigh"]
                },
                "PW": {
                    "country": "Palau",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "PY": {
                    "country": "Paraguay",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["paraguayLow", "paraguayHigh"]
                },
                "QA": {
                    "country": "Qatar",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["qatarLow", "qatarHigh"]
                },
                "RE": {
                    "country": "Reunion",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "RO": {
                    "country": "Romania",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["romaniaLow", "romaniaHigh"]
                },
                "RS": {
                    "country": "Serbia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["serbiaLow", "serbiaHigh", "serbiaNoKosovoLow", "serbiaNoKosovoHigh"]
                },
                "RU": {
                    "country": "Russian Federation",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["russiaLow", "russiaHigh", "russiaCrimeaLow", "russiaCrimeaHigh"]
                },
                "RW": {
                    "country": "Rwanda",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "SA": {
                    "country": "Saudi Arabia",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["saudiArabiaLow", "saudiArabiaHigh"]
                },
                "SB": {
                    "country": "Solomon Islands",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": ["solomonIslandsLow", "solomonIslandsHigh"]
                },
                "SC": {
                    "country": "Seychelles",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["seychellesLow", "seychellesHigh"]
                },
                "SD": {
                    "country": "Sudan",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["sudanLow", "sudanHigh"]
                },
                "SE": {
                    "country": "Sweden",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["swedenLow", "swedenHigh"]
                },
                "SG": {
                    "country": "Singapore",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["singaporeLow", "singaporeHigh"]
                },
                "SH": {
                    "country": "Saint Helena, Ascension and Tristan da Cunha",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["saintHelenaLow", "saintHelenaHigh"]
                },
                "SI": {
                    "country": "Slovenia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["sloveniaLow", "sloveniaHigh", "sloveniaRegionsLow", "sloveniaRegionsHigh"]
                },
                "SJ": {
                    "country": "Svalbard and Jan Mayen",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["svalbardLow", "svalbardHigh"]
                },
                "SK": {
                    "country": "Slovakia",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["slovakiaLow", "slovakiaHigh"]
                },
                "SL": {
                    "country": "Sierra Leone",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "SM": {
                    "country": "San Marino",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["sanMarinoLow", "sanMarinoHigh"]
                },
                "SN": {
                    "country": "Senegal",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["senegalLow", "senegalHigh"]
                },
                "SO": {
                    "country": "Somalia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["somaliaLow", "somaliaHigh"]
                },
                "SR": {
                    "country": "Suriname",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": []
                },
                "SS": {
                    "country": "South Sudan",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "ST": {
                    "country": "Sao Tome and Principe",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["saoTomePrincipeLow", "saoTomePrincipeHigh"]
                },
                "SV": {
                    "country": "El Salvador",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["elSalvadorLow", "elSalvadorHigh"]
                },
                "SX": {
                    "country": "Sint Maarten (Dutch Part)",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "SY": {
                    "country": "Syrian Arab Republic",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["syriaLow", "syriaHigh"]
                },
                "SZ": {
                    "country": "Swaziland",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["eswatiniLow", "eswatiniHigh"]
                },
                "TC": {
                    "country": "Turks and Caicos Islands",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "TD": {
                    "country": "Chad",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["chadLow", "chadHigh"]
                },
                "TF": {
                    "country": "French Southern Territories",
                    "continent_code": "AN",
                    "continent": "Antarctica",
                    "maps": []
                },
                "TG": {
                    "country": "Togo",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "TH": {
                    "country": "Thailand",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["thailandLow", "thailandHigh"]
                },
                "TJ": {
                    "country": "Tajikistan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["tajikistanLow", "tajikistanHigh"]
                },
                "TK": {
                    "country": "Tokelau",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "TL": {
                    "country": "Timor-Leste",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "TM": {
                    "country": "Turkmenistan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "TN": {
                    "country": "Tunisia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["tunisiaLow", "tunisiaHigh"]
                },
                "TO": {
                    "country": "Tonga",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "TR": {
                    "country": "Turkey",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["turkeyLow", "turkeyHigh"]
                },
                "TT": {
                    "country": "Trinidad and Tobago",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "TV": {
                    "country": "Tuvalu",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "TW": {
                    "country": "Taiwan, Province of China",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": []
                },
                "TZ": {
                    "country": "Tanzania, United Republic of",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["tanzaniaLow", "tanzaniaHigh"]
                },
                "UA": {
                    "country": "Ukraine",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["ukraineLow", "ukraineHigh"]
                },
                "UG": {
                    "country": "Uganda",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "UM": {
                    "country": "United States Minor Outlying Islands",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "US": {
                    "country": "United States",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["usaLow", "usaHigh", "usaTerritoriesLow", "usaTerritoriesHigh", "usaTerritories2Low", "usaTerritories2High"]
                },
                "UY": {
                    "country": "Uruguay",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": []
                },
                "UZ": {
                    "country": "Uzbekistan",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["uzbekinstanLow", "uzbekinstanHigh"]
                },
                "VA": {
                    "country": "Holy See (Vatican City State)",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["vaticanLow", "vaticanHigh"]
                },
                "VC": {
                    "country": "Saint Vincent and the Grenadines",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": ["saintVincentLow", "saintVincentHigh"]
                },
                "VE": {
                    "country": "Venezuela, Bolivarian Republic of",
                    "continent_code": "SA",
                    "continent": "South America",
                    "maps": ["venezuelaLow", "venezuelaHigh"]
                },
                "VG": {
                    "country": "Virgin Islands, British",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "VI": {
                    "country": "Virgin Islands, U.S.",
                    "continent_code": "NA",
                    "continent": "North America",
                    "maps": []
                },
                "VN": {
                    "country": "Viet Nam",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["vietnamLow", "vietnamHigh"]
                },
                "VU": {
                    "country": "Vanuatu",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "WF": {
                    "country": "Wallis and Futuna",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": []
                },
                "WS": {
                    "country": "Samoa",
                    "continent_code": "OC",
                    "continent": "Oceania",
                    "maps": ["samoaLow", "samoaHigh"]
                },
                "YE": {
                    "country": "Yemen",
                    "continent_code": "AS",
                    "continent": "Asia",
                    "maps": ["yemenLow", "yemenHigh"]
                },
                "YT": {
                    "country": "Mayotte",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": []
                },
                "ZA": {
                    "country": "South Africa",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["southAfricaLow", "southAfricaHigh"]
                },
                "ZM": {
                    "country": "Zambia",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["zambiaLow", "zambiaHigh"]
                },
                "ZW": {
                    "country": "Zimbabwe",
                    "continent_code": "AF",
                    "continent": "Africa",
                    "maps": ["zimbabweLow", "zimbabweHigh"]
                },
                "XK": {
                    "country": "Kosovo",
                    "continent_code": "EU",
                    "continent": "Europe",
                    "maps": ["kosovoLow", "kosovoHigh"]
                }
            };


            var continents = {
                "AF": 0,
                "AN": 1,
                "AS": 2,
                "EU": 3,
                "NA": 4,
                "OC": 5,
                "SA": 6
            }

            // Create map instance
            var chart = am4core.create("chartdiv", am4maps.MapChart);
            chart.projection = new am4maps.projections.Miller();

            // Create map polygon series for world map
            var worldSeries = chart.series.push(new am4maps.MapPolygonSeries());
            worldSeries.useGeodata = true;
            worldSeries.geodata = am4geodata_worldLow;
            worldSeries.exclude = ["AQ"];

            var worldPolygon = worldSeries.mapPolygons.template;
            worldPolygon.tooltipText = "{name}";
            worldPolygon.nonScalingStroke = true;
            worldPolygon.strokeOpacity = 0.5;
            worldPolygon.fill = am4core.color("#eee");
            worldPolygon.propertyFields.fill = "color";

            var hs = worldPolygon.states.create("hover");
            hs.properties.fill = chart.colors.getIndex(9);


            // Create country specific series (but hide it for now)
            var countrySeries = chart.series.push(new am4maps.MapPolygonSeries());
            countrySeries.useGeodata = true;
            countrySeries.hide();
            countrySeries.geodataSource.events.on("done", function(ev) {
                worldSeries.hide();
                countrySeries.show();
            });

            var countryPolygon = countrySeries.mapPolygons.template;
            countryPolygon.tooltipText = "{name}";
            countryPolygon.nonScalingStroke = true;
            countryPolygon.strokeOpacity = 0.5;
            countryPolygon.fill = am4core.color("#eee");

            var hs = countryPolygon.states.create("hover");
            hs.properties.fill = chart.colors.getIndex(9);

            // Set up click events
            worldPolygon.events.on("hit", function(ev) {
                ev.target.series.chart.zoomToMapObject(ev.target);
                var map = ev.target.dataItem.dataContext.map;
                if (map) {
                    ev.target.isHover = false;
                    countrySeries.geodataSource.url = "https://www.amcharts.com/lib/4/geodata/json/" + map + ".json";
                    countrySeries.geodataSource.load();
                }
            });

            // Set up data for countries
            var data = [];
            for (var id in countries) {
                if (countries.hasOwnProperty(id)) {
                    var country = countries[id];
                    if (country.maps.length) {
                        data.push({
                            id: id,
                            color: chart.colors.getIndex(continents[country.continent_code]),
                            map: country.maps[0]
                        });
                    }
                }
            }
            worldSeries.data = data;

            // Zoom control
            chart.zoomControl = new am4maps.ZoomControl();

            var homeButton = new am4core.Button();
            homeButton.events.on("hit", function() {
                worldSeries.show();
                countrySeries.hide();
                chart.goHome();
            });

            homeButton.icon = new am4core.Sprite();
            homeButton.padding(7, 5, 7, 5);
            homeButton.width = 30;
            homeButton.icon.path = "M16,8 L14,8 L14,16 L10,16 L10,10 L6,10 L6,16 L2,16 L2,8 L0,8 L8,0 L16,8 Z M16,8";
            homeButton.marginBottom = 10;
            homeButton.parent = chart.zoomControl;
            homeButton.insertBefore(chart.zoomControl.plusButton);

        });
    });
}


// calender js
if (jQuery('#calendar').length) {
    document.addEventListener('DOMContentLoaded', function() {
        var calendarEl = document.getElementById('calendar');
        var calendar = new FullCalendar.Calendar(calendarEl, {
            plugins: ['dayGrid']
        });
        calendar.render();
    });
}

// calender 1 js
if (jQuery('#calendar1').length) {
    document.addEventListener('DOMContentLoaded', function() {
        var calendarEl = document.getElementById('calendar1');

        var calendar1 = new FullCalendar.Calendar(calendarEl, {
            plugins: ['timeGrid', 'dayGrid', 'list'],
            timeZone: 'UTC',
            defaultView: 'dayGridMonth',
            header: {
                left: 'prev,next today',
                center: 'title',
                right: 'dayGridMonth,timeGridWeek,timeGridDay,listWeek'
            },
            events: [{
                title: 'All Day Event',
                start: '2019-12-01',
                color: '#fc9919'
            }, {
                title: 'Long Event',
                start: '2019-12-07',
                end: '2019-12-10',
                color: '#ffc107' // override!
            }, {
                groupId: '999',
                title: 'Repeating Event',
                start: '2019-12-09T16:00:00',
                color: '#17a2b8'
            }, {
                groupId: '999',
                title: 'Repeating Event',
                start: '2019-12-16T16:00:00',
                color: '#17a2b8'
            }, {
                title: 'Conference',
                start: '2019-12-11',
                end: '2019-12-13',
                color: '#27e3f4' // override!
            }, {
                title: 'Meeting',
                start: '2019-12-12T10:30:00',
                end: '2019-12-12T12:30:00',
                color: '#089bab'
            }, {
                title: 'Lunch',
                start: '2019-12-12T12:00:00',
                color: '#777D74'
            }, {
                title: 'Meeting',
                start: '2019-12-12T14:30:00',
                color: '#089bab'
            }, {
                title: 'Birthday Party',
                start: '2019-12-28T07:00:00',
                color: '#28a745'
            }, {
                title: 'Meeting',
                start: '2020-01-12T14:30:00',
                color: '#089bab'
            }, {
                title: 'Birthday Party',
                start: '2020-01-02T07:00:00',
                color: '#28a745'
            }, {
                title: 'Click for Google',
                url: 'http://google.com/',
                start: '2020-01-25'
            }, {
                title: 'Birthday Party',
                start: '2020-01-13T07:00:00',
                color: '#28a745'
            }, {
                title: 'Click for Google',
                url: 'http://google.com/',
                start: '2019-12-28'
            }, {
                title: 'Meeting',
                start: '2020-01-12T14:30:00',
                color: '#089bab'
            }, {
                title: 'Birthday Party',
                start: '2020-01-13T07:00:00',
                color: '#28a745'
            }, {
                title: 'Click for Google',
                url: 'http://google.com/',
                start: '2020-01-28'
            }, {
                title: 'All Day Event',
                start: '2020-02-01',
                color: '#fc9919'
            }, {
                title: 'Long Event',
                start: '2020-02-07',
                end: '2020-02-10',
                color: '#ffc107' // override!
            }, {
                groupId: '999',
                title: 'Repeating Event',
                start: '2020-02-09T16:00:00',
                color: '#17a2b8'
            }, {
                groupId: '999',
                title: 'Repeating Event',
                start: '2020-02-16T16:00:00',
                color: '#17a2b8'
            }]
        });


        calendar1.render();
    });
}

// Chart Apex js
if (jQuery('#apex-basic').length) {
    var options = {
        chart: {
            height: 350,
            type: 'line',
            zoom: {
                enabled: false
            }
        },
        colors: ['#089bab'],
        series: [{
            name: "Desktops",
            data: [10, 41, 35, 51, 49, 62, 69, 91, 148]
        }],
        dataLabels: {
            enabled: false
        },
        stroke: {
            curve: 'straight'
        },
        title: {
            text: 'Product Trends by Month',
            align: 'left'
        },
        grid: {
            row: {
                colors: ['#f3f3f3', 'transparent'], // takes an array which will be repeated on columns
                opacity: 0.5
            },
        },
        xaxis: {
            categories: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep'],
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-basic"),
        options
    );

    chart.render();
}
if (jQuery('#apex-line-area').length) {
    var options = {
        chart: {
            height: 350,
            type: 'area',
        },
        dataLabels: {
            enabled: false
        },
        stroke: {
            curve: 'smooth'
        },
        colors: ['#089bab', '#FC9F5B'],
        series: [{
            name: 'series1',
            data: [31, 40, 28, 51, 42, 109, 100]
        }, {
            name: 'series2',
            data: [11, 32, 45, 32, 34, 52, 41]
        }],

        xaxis: {
            type: 'datetime',
            categories: ["2018-09-19T00:00:00", "2018-09-19T01:30:00", "2018-09-19T02:30:00", "2018-09-19T03:30:00", "2018-09-19T04:30:00", "2018-09-19T05:30:00", "2018-09-19T06:30:00"],
        },
        tooltip: {
            x: {
                format: 'dd/MM/yy HH:mm'
            },
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-line-area"),
        options
    );

    chart.render();
}
if (jQuery('#apex-bar').length) {
    var options = {
        chart: {
            height: 350,
            type: 'bar',
        },
        colors: ['#089bab'],
        plotOptions: {
            bar: {
                horizontal: true,
            }
        },
        dataLabels: {
            enabled: false
        },
        series: [{
            data: [470, 540, 580, 690, 1100, 1200, 1380]
        }],
        xaxis: {
            categories: ['Netherlands', 'Italy', 'France', 'Japan', 'United States', 'China', 'Germany'],
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-bar"),
        options
    );

    chart.render();
}
if (jQuery('#apex-column').length) {
    var options = {
        chart: {
            height: 350,
            type: 'bar',
        },
        plotOptions: {
            bar: {
                horizontal: false,
                columnWidth: '55%',
                endingShape: 'rounded'
            },
        },
        dataLabels: {
            enabled: false
        },
        stroke: {
            show: true,
            width: 2,
            colors: ['transparent']
        },
        colors: ['#089bab', '#FC9F5B', '#e64141'],
        series: [{
            name: 'Net Profit',
            data: [44, 55, 57, 56, 61, 58]
        }, {
            name: 'Revenue',
            data: [76, 85, 101, 98, 87, 105]
        }, {
            name: 'Free Cash Flow',
            data: [35, 41, 36, 26, 45, 48]
        }],
        xaxis: {
            categories: ['Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul'],
        },
        yaxis: {
            title: {
                text: '$ (thousands)'
            }
        },
        fill: {
            opacity: 1

        },
        tooltip: {
            y: {
                formatter: function(val) {
                    return "$ " + val + " thousands"
                }
            }
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-column"),
        options
    );

    chart.render();
}

if (jQuery('#apex-mixed-chart').length) {
    var options = {
        chart: {
            height: 350,
            type: 'line',
            stacked: false,
        },
        stroke: {
            width: [0, 2, 5],
            curve: 'smooth'
        },
        plotOptions: {
            bar: {
                columnWidth: '50%'
            }
        },
        colors: ['#089bab','#e64141' ,'#FC9F5B'],
        series: [{
            name: 'Facebook',
            type: 'column',
            data: [23, 11, 22, 27, 13, 22, 37, 21, 44, 22, 30]
        }, {
            name: 'Vine',
            type: 'area',
            data: [44, 55, 41, 67, 22, 43, 21, 41, 56, 27, 43]
        }, {
            name: 'Dribbble',
            type: 'line',
            data: [30, 25, 36, 30, 45, 35, 64, 52, 59, 36, 39]
        }],
        fill: {
            opacity: [0.85, 0.25, 1],
            gradient: {
                inverseColors: false,
                shade: 'light',
                type: "vertical",
                opacityFrom: 0.85,
                opacityTo: 0.55,
                stops: [0, 100, 100, 100]
            }
        },
        labels: ['01/01/2003', '02/01/2003', '03/01/2003', '04/01/2003', '05/01/2003', '06/01/2003', '07/01/2003', '08/01/2003', '09/01/2003', '10/01/2003', '11/01/2003'],
        markers: {
            size: 0
        },
        xaxis: {
            type: 'datetime'
        },
        yaxis: {
            min: 0
        },
        tooltip: {
            shared: true,
            intersect: false,
            y: {
                formatter: function(y) {
                    if (typeof y !== "undefined") {
                        return y.toFixed(0) + " views";
                    }
                    return y;

                }
            }
        },
        legend: {
            labels: {
                useSeriesColors: true
            },
            markers: {
                customHTML: [
                    function() {
                        return ''
                    },
                    function() {
                        return ''
                    },
                    function() {
                        return ''
                    }
                ]
            }
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-mixed-chart"),
        options
    );

    chart.render();
}

if (jQuery('#apex-candlestick-chart').length) {
    var options = {
        chart: {
            height: 350,
            type: 'candlestick',
        },
        series: [{
            data: [{
                x: new Date(1538778600000),
                y: [6629.81, 6650.5, 6623.04, 6633.33]
            }, {
                x: new Date(1538780400000),
                y: [6632.01, 6643.59, 6620, 6630.11]
            }, {
                x: new Date(1538782200000),
                y: [6630.71, 6648.95, 6623.34, 6635.65]
            }, {
                x: new Date(1538784000000),
                y: [6635.65, 6651, 6629.67, 6638.24]
            }, {
                x: new Date(1538785800000),
                y: [6638.24, 6640, 6620, 6624.47]
            }, {
                x: new Date(1538787600000),
                y: [6624.53, 6636.03, 6621.68, 6624.31]
            }, {
                x: new Date(1538789400000),
                y: [6624.61, 6632.2, 6617, 6626.02]
            }, {
                x: new Date(1538791200000),
                y: [6627, 6627.62, 6584.22, 6603.02]
            }, {
                x: new Date(1538793000000),
                y: [6605, 6608.03, 6598.95, 6604.01]
            }, {
                x: new Date(1538794800000),
                y: [6604.5, 6614.4, 6602.26, 6608.02]
            }, {
                x: new Date(1538796600000),
                y: [6608.02, 6610.68, 6601.99, 6608.91]
            }, {
                x: new Date(1538798400000),
                y: [6608.91, 6618.99, 6608.01, 6612]
            }, {
                x: new Date(1538800200000),
                y: [6612, 6615.13, 6605.09, 6612]
            }, {
                x: new Date(1538802000000),
                y: [6612, 6624.12, 6608.43, 6622.95]
            }, {
                x: new Date(1538803800000),
                y: [6623.91, 6623.91, 6615, 6615.67]
            }, {
                x: new Date(1538805600000),
                y: [6618.69, 6618.74, 6610, 6610.4]
            }, {
                x: new Date(1538807400000),
                y: [6611, 6622.78, 6610.4, 6614.9]
            }, {
                x: new Date(1538809200000),
                y: [6614.9, 6626.2, 6613.33, 6623.45]
            }, {
                x: new Date(1538811000000),
                y: [6623.48, 6627, 6618.38, 6620.35]
            }, {
                x: new Date(1538812800000),
                y: [6619.43, 6620.35, 6610.05, 6615.53]
            }, {
                x: new Date(1538814600000),
                y: [6615.53, 6617.93, 6610, 6615.19]
            }, {
                x: new Date(1538816400000),
                y: [6615.19, 6621.6, 6608.2, 6620]
            }, {
                x: new Date(1538818200000),
                y: [6619.54, 6625.17, 6614.15, 6620]
            }, {
                x: new Date(1538820000000),
                y: [6620.33, 6634.15, 6617.24, 6624.61]
            }, {
                x: new Date(1538821800000),
                y: [6625.95, 6626, 6611.66, 6617.58]
            }, {
                x: new Date(1538823600000),
                y: [6619, 6625.97, 6595.27, 6598.86]
            }, {
                x: new Date(1538825400000),
                y: [6598.86, 6598.88, 6570, 6587.16]
            }, {
                x: new Date(1538827200000),
                y: [6588.86, 6600, 6580, 6593.4]
            }, {
                x: new Date(1538829000000),
                y: [6593.99, 6598.89, 6585, 6587.81]
            }, {
                x: new Date(1538830800000),
                y: [6587.81, 6592.73, 6567.14, 6578]
            }, {
                x: new Date(1538832600000),
                y: [6578.35, 6581.72, 6567.39, 6579]
            }, {
                x: new Date(1538834400000),
                y: [6579.38, 6580.92, 6566.77, 6575.96]
            }, {
                x: new Date(1538836200000),
                y: [6575.96, 6589, 6571.77, 6588.92]
            }, {
                x: new Date(1538838000000),
                y: [6588.92, 6594, 6577.55, 6589.22]
            }, {
                x: new Date(1538839800000),
                y: [6589.3, 6598.89, 6589.1, 6596.08]
            }, {
                x: new Date(1538841600000),
                y: [6597.5, 6600, 6588.39, 6596.25]
            }, {
                x: new Date(1538843400000),
                y: [6598.03, 6600, 6588.73, 6595.97]
            }, {
                x: new Date(1538845200000),
                y: [6595.97, 6602.01, 6588.17, 6602]
            }, {
                x: new Date(1538847000000),
                y: [6602, 6607, 6596.51, 6599.95]
            }, {
                x: new Date(1538848800000),
                y: [6600.63, 6601.21, 6590.39, 6591.02]
            }, {
                x: new Date(1538850600000),
                y: [6591.02, 6603.08, 6591, 6591]
            }, {
                x: new Date(1538852400000),
                y: [6591, 6601.32, 6585, 6592]
            }, {
                x: new Date(1538854200000),
                y: [6593.13, 6596.01, 6590, 6593.34]
            }, {
                x: new Date(1538856000000),
                y: [6593.34, 6604.76, 6582.63, 6593.86]
            }, {
                x: new Date(1538857800000),
                y: [6593.86, 6604.28, 6586.57, 6600.01]
            }, {
                x: new Date(1538859600000),
                y: [6601.81, 6603.21, 6592.78, 6596.25]
            }, {
                x: new Date(1538861400000),
                y: [6596.25, 6604.2, 6590, 6602.99]
            }, {
                x: new Date(1538863200000),
                y: [6602.99, 6606, 6584.99, 6587.81]
            }, {
                x: new Date(1538865000000),
                y: [6587.81, 6595, 6583.27, 6591.96]
            }, {
                x: new Date(1538866800000),
                y: [6591.97, 6596.07, 6585, 6588.39]
            }, {
                x: new Date(1538868600000),
                y: [6587.6, 6598.21, 6587.6, 6594.27]
            }, {
                x: new Date(1538870400000),
                y: [6596.44, 6601, 6590, 6596.55]
            }, {
                x: new Date(1538872200000),
                y: [6598.91, 6605, 6596.61, 6600.02]
            }, {
                x: new Date(1538874000000),
                y: [6600.55, 6605, 6589.14, 6593.01]
            }, {
                x: new Date(1538875800000),
                y: [6593.15, 6605, 6592, 6603.06]
            }, ]
        }],
        title: {
            text: 'CandleStick Chart',
            align: 'left'
        },
        xaxis: {
            type: 'datetime'
        },
        yaxis: {
            tooltip: {
                enabled: true
            }
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-candlestick-chart"),
        options
    );
    chart.render();
}

if (jQuery('#apex-bubble-chart').length) {
    function generateData(baseval, count, yrange) {
        var i = 0;
        var series = [];
        while (i < count) {
            var y = Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min;
            var z = Math.floor(Math.random() * (75 - 15 + 1)) + 15;

            series.push([baseval, y, z]);
            baseval += 86400000;
            i++;
        }
        return series;
    }


    var options = {
        chart: {
            height: 350,
            type: 'bubble',
        },
        dataLabels: {
            enabled: false
        },
        series: [{
            name: 'Product1',
            data: generateData(new Date('11 Feb 2017 GMT').getTime(), 20, {
                min: 10,
                max: 40
            })
        }, {
            name: 'Product2',
            data: generateData(new Date('11 Feb 2017 GMT').getTime(), 20, {
                min: 10,
                max: 40
            })
        }, {
            name: 'Product3',
            data: generateData(new Date('11 Feb 2017 GMT').getTime(), 20, {
                min: 10,
                max: 40
            })
        }],
        fill: {
            type: 'gradient',
        },
        colors: ['#089bab', '#FC9F5B', '#e64141'],
        title: {
            text: '3D Bubble Chart'
        },
        xaxis: {
            tickAmount: 12,
            type: 'datetime',

            labels: {
                rotate: 0,
            }
        },
        yaxis: {
            max: 40
        },
        theme: {
            palette: 'palette2'
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-bubble-chart"),
        options
    );

    chart.render();
}

if (jQuery('#apex-scatter-chart').length) {
    var options = {
        chart: {
            height: 350,
            type: 'scatter',
            zoom: {
                enabled: true,
                type: 'xy'
            }
        },
        colors: ['#089bab', '#e64141', '#FC9F5B'],
        series: [{
            name: "SAMPLE A",
            data: [
                [16.4, 5.4],
                [21.7, 2],
                [10.9, 0],
                [10.9, 8.2],
                [16.4, 0],
                [16.4, 1.8],
                [13.6, 0.3],
                [13.6, 0],
                [29.9, 0],
                [27.1, 2.3],
                [16.4, 0],
                [13.6, 3.7],
                [10.9, 5.2],
                [16.4, 6.5],
                [10.9, 0],
                [24.5, 7.1],
                [10.9, 0],
                [8.1, 4.7]
            ]
        }, {
            name: "SAMPLE B",
            data: [
                [36.4, 13.4],
                [1.7, 11],
                [1.9, 9],
                [1.9, 13.2],
                [1.4, 7],
                [6.4, 8.8],
                [3.6, 4.3],
                [1.6, 10],
                [9.9, 2],
                [7.1, 15],
                [1.4, 0],
                [3.6, 13.7],
                [1.9, 15.2],
                [6.4, 16.5],
                [0.9, 10],
                [4.5, 17.1],
                [10.9, 10],
                [0.1, 14.7]
            ]
        }, {
            name: "SAMPLE C",
            data: [
                [21.7, 3],
                [23.6, 3.5],
                [28, 4],
                [27.1, 0.3],
                [16.4, 4],
                [13.6, 0],
                [19, 5],
                [22.4, 3],
                [24.5, 3],
                [32.6, 3],
                [27.1, 4],
                [29.6, 6],
                [31.6, 8],
                [21.6, 5],
                [20.9, 4],
                [22.4, 0],
                [32.6, 10.3],
                [29.7, 20.8]
            ]
        }],
        xaxis: {
            tickAmount: 5,
            labels: {
                formatter: function(val) {
                    return parseFloat(val).toFixed(1)
                }
            }
        },
        yaxis: {
            tickAmount: 5
        }
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-scatter-chart"),
        options
    );

    chart.render();
}
if (jQuery('#apex-radialbar-chart').length) {
    var options = {
        chart: {
            height: 350,
            type: 'radialBar',
        },
        plotOptions: {
            radialBar: {
                dataLabels: {
                    name: {
                        fontSize: '22px',
                    },
                    value: {
                        fontSize: '16px',
                    },
                    total: {
                        show: true,
                        label: 'Total',
                        formatter: function(w) {
                            // By default this function returns the average of all series. The below is just an example to show the use of custom formatter function
                            return 249
                        }
                    }
                }
            }
        },
        series: [44, 55, 67, 83],
        labels: ['Apples', 'Oranges', 'Bananas', 'Berries'],
        colors: ['#089bab', '#FC9F5B', '#75DDDD', '#ffb57e'],

    }

    var chart = new ApexCharts(
        document.querySelector("#apex-radialbar-chart"),
        options
    );

    chart.render();
}
if (jQuery('#apex-pie-chart').length) {
    var options = {
        chart: {
            width: 380,
            type: 'pie',
        },
        labels: ['Team A', 'Team B', 'Team C', 'Team D', 'Team E'],
        series: [44, 55, 13, 43, 22],
        colors: ['#089bab', '#FC9F5B', '#f26361', '#57de53', '#61e2fc'],
        responsive: [{
            breakpoint: 480,
            options: {
                chart: {
                    width: 200
                },
                legend: {
                    position: 'bottom'
                }
            }
        }]
    }

    var chart = new ApexCharts(
        document.querySelector("#apex-pie-chart"),
        options
    );

    chart.render();
}

// Am Charts 

if (jQuery('#am-simple-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-simple-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab"), ];

        // Add data
        chart.data = [{
            "country": "USA",
            "visits": 2025
        }, {
            "country": "China",
            "visits": 1882
        }, {
            "country": "Japan",
            "visits": 1809
        }, {
            "country": "Germany",
            "visits": 1322
        }, {
            "country": "UK",
            "visits": 1122
        }, {
            "country": "France",
            "visits": 1114
        }];

        // Create axes

        var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "country";
        categoryAxis.renderer.grid.template.location = 0;
        categoryAxis.renderer.minGridDistance = 30;

        categoryAxis.renderer.labels.template.adapter.add("dy", function(dy, target) {
            if (target.dataItem && target.dataItem.index & 2 == 2) {
                return dy + 25;
            }
            return dy;
        });

        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());

        // Create series
        var series = chart.series.push(new am4charts.ColumnSeries());
        series.dataFields.valueY = "visits";
        series.dataFields.categoryX = "country";
        series.name = "Visits";
        series.columns.template.tooltipText = "{categoryX}: [bold]{valueY}[/]";
        series.columns.template.fillOpacity = .8;

        var columnTemplate = series.columns.template;
        columnTemplate.strokeWidth = 2;
        columnTemplate.strokeOpacity = 1;

    }); // end am4core.ready()
}

if (jQuery('#am-columnlinr-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-columnlinr-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab")];

        // Export
        chart.exporting.menu = new am4core.ExportMenu();

        // Data for both series
        var data = [{
            "year": "2009",
            "income": 23.5,
            "expenses": 21.1
        }, {
            "year": "2010",
            "income": 26.2,
            "expenses": 30.5
        }, {
            "year": "2011",
            "income": 30.1,
            "expenses": 34.9
        }, {
            "year": "2012",
            "income": 29.5,
            "expenses": 31.1
        }, {
            "year": "2013",
            "income": 30.6,
            "expenses": 28.2,
            "lineDash": "5,5",
        }, {
            "year": "2014",
            "income": 34.1,
            "expenses": 32.9,
            "strokeWidth": 1,
            "columnDash": "5,5",
            "fillOpacity": 0.2,
            "additional": "(projection)"
        }];

        /* Create axes */
        var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "year";
        categoryAxis.renderer.minGridDistance = 30;

        /* Create value axis */
        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());

        /* Create series */
        var columnSeries = chart.series.push(new am4charts.ColumnSeries());
        columnSeries.name = "Income";
        columnSeries.dataFields.valueY = "income";
        columnSeries.dataFields.categoryX = "year";

        columnSeries.columns.template.tooltipText = "[#fff font-size: 15px]{name} in {categoryX}:\n[/][#fff font-size: 20px]{valueY}[/] [#fff]{additional}[/]"
        columnSeries.columns.template.propertyFields.fillOpacity = "fillOpacity";
        columnSeries.columns.template.propertyFields.stroke = "stroke";
        columnSeries.columns.template.propertyFields.strokeWidth = "strokeWidth";
        columnSeries.columns.template.propertyFields.strokeDasharray = "columnDash";
        columnSeries.tooltip.label.textAlign = "middle";

        var lineSeries = chart.series.push(new am4charts.LineSeries());
        lineSeries.name = "Expenses";
        lineSeries.dataFields.valueY = "expenses";
        lineSeries.dataFields.categoryX = "year";

        lineSeries.stroke = am4core.color("#089bab");
        lineSeries.strokeWidth = 3;
        lineSeries.propertyFields.strokeDasharray = "lineDash";
        lineSeries.tooltip.label.textAlign = "middle";

        var bullet = lineSeries.bullets.push(new am4charts.Bullet());
        bullet.fill = am4core.color("#fdd400"); // tooltips grab fill from parent by default
        bullet.tooltipText = "[#fff font-size: 15px]{name} in {categoryX}:\n[/][#fff font-size: 20px]{valueY}[/] [#fff]{additional}[/]"
        var circle = bullet.createChild(am4core.Circle);
        circle.radius = 4;
        circle.fill = am4core.color("#fff");
        circle.strokeWidth = 3;

        chart.data = data;

    });
}

if (jQuery('#am-stackedcolumn-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-stackedcolumn-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab"),
            am4core.color("#FC9F5B"),
            am4core.color("#e64141")
        ];


        // Add data
        chart.data = [{
            "year": "2016",
            "europe": 2.5,
            "namerica": 2.5,
            "asia": 2.1,
            "lamerica": 0.3,
            "meast": 0.2
        }, {
            "year": "2017",
            "europe": 2.6,
            "namerica": 2.7,
            "asia": 2.2,
            "lamerica": 0.3,
            "meast": 0.3
        }, {
            "year": "2018",
            "europe": 2.8,
            "namerica": 2.9,
            "asia": 2.4,
            "lamerica": 0.3,
            "meast": 0.3
        }];

        // Create axes
        var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "year";
        categoryAxis.renderer.grid.template.location = 0;


        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
        valueAxis.renderer.inside = true;
        valueAxis.renderer.labels.template.disabled = true;
        valueAxis.min = 0;

        // Create series
        function createSeries(field, name) {

            // Set up series
            var series = chart.series.push(new am4charts.ColumnSeries());
            series.name = name;
            series.dataFields.valueY = field;
            series.dataFields.categoryX = "year";
            series.sequencedInterpolation = true;

            // Make it stacked
            series.stacked = true;

            // Configure columns
            series.columns.template.width = am4core.percent(60);
            series.columns.template.tooltipText = "[bold]{name}[/]\n[font-size:14px]{categoryX}: {valueY}";

            // Add label
            var labelBullet = series.bullets.push(new am4charts.LabelBullet());
            labelBullet.label.text = "{valueY}";
            labelBullet.locationY = 0.5;

            return series;
        }

        createSeries("europe", "Europe");
        createSeries("namerica", "North America");
        createSeries("asia", "Asia-Pacific");

        // Legend
        chart.legend = new am4charts.Legend();

    }); // end am4core.ready()
}

if (jQuery('#am-barline-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        var chart = am4core.create("am-barline-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab"),
            am4core.color("#FC9F5B")
        ];

        chart.data = [{
            "year": "2005",
            "income": 23.5,
            "expenses": 18.1
        }, {
            "year": "2006",
            "income": 26.2,
            "expenses": 22.8
        }, {
            "year": "2007",
            "income": 30.1,
            "expenses": 23.9
        }, {
            "year": "2008",
            "income": 29.5,
            "expenses": 25.1
        }, {
            "year": "2009",
            "income": 24.6,
            "expenses": 25
        }];

        //create category axis for years
        var categoryAxis = chart.yAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "year";
        categoryAxis.renderer.inversed = true;
        categoryAxis.renderer.grid.template.location = 0;

        //create value axis for income and expenses
        var valueAxis = chart.xAxes.push(new am4charts.ValueAxis());
        valueAxis.renderer.opposite = true;


        //create columns
        var series = chart.series.push(new am4charts.ColumnSeries());
        series.dataFields.categoryY = "year";
        series.dataFields.valueX = "income";
        series.name = "Income";
        series.columns.template.fillOpacity = 0.5;
        series.columns.template.strokeOpacity = 0;
        series.tooltipText = "Income in {categoryY}: {valueX.value}";

        //create line
        var lineSeries = chart.series.push(new am4charts.LineSeries());
        lineSeries.dataFields.categoryY = "year";
        lineSeries.dataFields.valueX = "expenses";
        lineSeries.name = "Expenses";
        lineSeries.strokeWidth = 3;
        lineSeries.tooltipText = "Expenses in {categoryY}: {valueX.value}";

        //add bullets
        var circleBullet = lineSeries.bullets.push(new am4charts.CircleBullet());
        circleBullet.circle.fill = am4core.color("#fff");
        circleBullet.circle.strokeWidth = 2;

        //add chart cursor
        chart.cursor = new am4charts.XYCursor();
        chart.cursor.behavior = "zoomY";

        //add legend
        chart.legend = new am4charts.Legend();

    }); // end am4core.ready()
}

if (jQuery('#am-datedata-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-datedata-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab")];

        // Add data
        chart.data = [{
            "date": "2012-07-27",
            "value": 13
        }, {
            "date": "2012-07-28",
            "value": 11
        }, {
            "date": "2012-07-29",
            "value": 15
        }, {
            "date": "2012-07-30",
            "value": 16
        }, {
            "date": "2012-07-31",
            "value": 18
        }, {
            "date": "2012-08-01",
            "value": 13
        }, {
            "date": "2012-08-02",
            "value": 22
        }, {
            "date": "2012-08-03",
            "value": 23
        }, {
            "date": "2012-08-04",
            "value": 20
        }, {
            "date": "2012-08-05",
            "value": 17
        }, {
            "date": "2012-08-06",
            "value": 16
        }, {
            "date": "2012-08-07",
            "value": 18
        }, {
            "date": "2012-08-08",
            "value": 21
        }, {
            "date": "2012-08-09",
            "value": 26
        }, {
            "date": "2012-08-10",
            "value": 24
        }, {
            "date": "2012-08-11",
            "value": 29
        }, {
            "date": "2012-08-12",
            "value": 32
        }, {
            "date": "2012-08-13",
            "value": 18
        }, {
            "date": "2012-08-14",
            "value": 24
        }, {
            "date": "2012-08-15",
            "value": 22
        }, {
            "date": "2012-08-16",
            "value": 18
        }, {
            "date": "2012-08-17",
            "value": 19
        }, {
            "date": "2012-08-18",
            "value": 14
        }, {
            "date": "2012-08-19",
            "value": 15
        }, {
            "date": "2012-08-20",
            "value": 12
        }, {
            "date": "2012-08-21",
            "value": 8
        }, {
            "date": "2012-08-22",
            "value": 9
        }, {
            "date": "2012-08-23",
            "value": 8
        }, {
            "date": "2012-08-24",
            "value": 7
        }, {
            "date": "2012-08-25",
            "value": 5
        }, {
            "date": "2012-08-26",
            "value": 11
        }, {
            "date": "2012-08-27",
            "value": 13
        }, {
            "date": "2012-08-28",
            "value": 18
        }, {
            "date": "2012-08-29",
            "value": 20
        }, {
            "date": "2012-08-30",
            "value": 29
        }, {
            "date": "2012-08-31",
            "value": 33
        }, {
            "date": "2012-09-01",
            "value": 42
        }, {
            "date": "2012-09-02",
            "value": 35
        }, {
            "date": "2012-09-03",
            "value": 31
        }, {
            "date": "2012-09-04",
            "value": 47
        }, {
            "date": "2012-09-05",
            "value": 52
        }, {
            "date": "2012-09-06",
            "value": 46
        }, {
            "date": "2012-09-07",
            "value": 41
        }, {
            "date": "2012-09-08",
            "value": 43
        }, {
            "date": "2012-09-09",
            "value": 40
        }, {
            "date": "2012-09-10",
            "value": 39
        }, {
            "date": "2012-09-11",
            "value": 34
        }, {
            "date": "2012-09-12",
            "value": 29
        }, {
            "date": "2012-09-13",
            "value": 34
        }, {
            "date": "2012-09-14",
            "value": 37
        }, {
            "date": "2012-09-15",
            "value": 42
        }, {
            "date": "2012-09-16",
            "value": 49
        }, {
            "date": "2012-09-17",
            "value": 46
        }, {
            "date": "2012-09-18",
            "value": 47
        }, {
            "date": "2012-09-19",
            "value": 55
        }, {
            "date": "2012-09-20",
            "value": 59
        }, {
            "date": "2012-09-21",
            "value": 58
        }, {
            "date": "2012-09-22",
            "value": 57
        }, {
            "date": "2012-09-23",
            "value": 61
        }, {
            "date": "2012-09-24",
            "value": 59
        }, {
            "date": "2012-09-25",
            "value": 67
        }, {
            "date": "2012-09-26",
            "value": 65
        }, {
            "date": "2012-09-27",
            "value": 61
        }, {
            "date": "2012-09-28",
            "value": 66
        }, {
            "date": "2012-09-29",
            "value": 69
        }, {
            "date": "2012-09-30",
            "value": 71
        }, {
            "date": "2012-10-01",
            "value": 67
        }, {
            "date": "2012-10-02",
            "value": 63
        }, {
            "date": "2012-10-03",
            "value": 46
        }, {
            "date": "2012-10-04",
            "value": 32
        }, {
            "date": "2012-10-05",
            "value": 21
        }, {
            "date": "2012-10-06",
            "value": 18
        }, {
            "date": "2012-10-07",
            "value": 21
        }, {
            "date": "2012-10-08",
            "value": 28
        }, {
            "date": "2012-10-09",
            "value": 27
        }, {
            "date": "2012-10-10",
            "value": 36
        }, {
            "date": "2012-10-11",
            "value": 33
        }, {
            "date": "2012-10-12",
            "value": 31
        }, {
            "date": "2012-10-13",
            "value": 30
        }, {
            "date": "2012-10-14",
            "value": 34
        }, {
            "date": "2012-10-15",
            "value": 38
        }, {
            "date": "2012-10-16",
            "value": 37
        }, {
            "date": "2012-10-17",
            "value": 44
        }, {
            "date": "2012-10-18",
            "value": 49
        }, {
            "date": "2012-10-19",
            "value": 53
        }, {
            "date": "2012-10-20",
            "value": 57
        }, {
            "date": "2012-10-21",
            "value": 60
        }, {
            "date": "2012-10-22",
            "value": 61
        }, {
            "date": "2012-10-23",
            "value": 69
        }, {
            "date": "2012-10-24",
            "value": 67
        }, {
            "date": "2012-10-25",
            "value": 72
        }, {
            "date": "2012-10-26",
            "value": 77
        }, {
            "date": "2012-10-27",
            "value": 75
        }, {
            "date": "2012-10-28",
            "value": 70
        }, {
            "date": "2012-10-29",
            "value": 72
        }, {
            "date": "2012-10-30",
            "value": 70
        }, {
            "date": "2012-10-31",
            "value": 72
        }, {
            "date": "2012-11-01",
            "value": 73
        }, {
            "date": "2012-11-02",
            "value": 67
        }, {
            "date": "2012-11-03",
            "value": 68
        }, {
            "date": "2012-11-04",
            "value": 65
        }, {
            "date": "2012-11-05",
            "value": 71
        }, {
            "date": "2012-11-06",
            "value": 75
        }, {
            "date": "2012-11-07",
            "value": 74
        }, {
            "date": "2012-11-08",
            "value": 71
        }, {
            "date": "2012-11-09",
            "value": 76
        }, {
            "date": "2012-11-10",
            "value": 77
        }, {
            "date": "2012-11-11",
            "value": 81
        }, {
            "date": "2012-11-12",
            "value": 83
        }, {
            "date": "2012-11-13",
            "value": 80
        }, {
            "date": "2012-11-14",
            "value": 81
        }, {
            "date": "2012-11-15",
            "value": 87
        }, {
            "date": "2012-11-16",
            "value": 82
        }, {
            "date": "2012-11-17",
            "value": 86
        }, {
            "date": "2012-11-18",
            "value": 80
        }, {
            "date": "2012-11-19",
            "value": 87
        }, {
            "date": "2012-11-20",
            "value": 83
        }, {
            "date": "2012-11-21",
            "value": 85
        }, {
            "date": "2012-11-22",
            "value": 84
        }, {
            "date": "2012-11-23",
            "value": 82
        }, {
            "date": "2012-11-24",
            "value": 73
        }, {
            "date": "2012-11-25",
            "value": 71
        }, {
            "date": "2012-11-26",
            "value": 75
        }, {
            "date": "2012-11-27",
            "value": 79
        }, {
            "date": "2012-11-28",
            "value": 70
        }, {
            "date": "2012-11-29",
            "value": 73
        }, {
            "date": "2012-11-30",
            "value": 61
        }, {
            "date": "2012-12-01",
            "value": 62
        }, {
            "date": "2012-12-02",
            "value": 66
        }, {
            "date": "2012-12-03",
            "value": 65
        }, {
            "date": "2012-12-04",
            "value": 73
        }, {
            "date": "2012-12-05",
            "value": 79
        }, {
            "date": "2012-12-06",
            "value": 78
        }, {
            "date": "2012-12-07",
            "value": 78
        }, {
            "date": "2012-12-08",
            "value": 78
        }, {
            "date": "2012-12-09",
            "value": 74
        }, {
            "date": "2012-12-10",
            "value": 73
        }, {
            "date": "2012-12-11",
            "value": 75
        }, {
            "date": "2012-12-12",
            "value": 70
        }, {
            "date": "2012-12-13",
            "value": 77
        }, {
            "date": "2012-12-14",
            "value": 67
        }, {
            "date": "2012-12-15",
            "value": 62
        }, {
            "date": "2012-12-16",
            "value": 64
        }, {
            "date": "2012-12-17",
            "value": 61
        }, {
            "date": "2012-12-18",
            "value": 59
        }, {
            "date": "2012-12-19",
            "value": 53
        }, {
            "date": "2012-12-20",
            "value": 54
        }, {
            "date": "2012-12-21",
            "value": 56
        }, {
            "date": "2012-12-22",
            "value": 59
        }, {
            "date": "2012-12-23",
            "value": 58
        }, {
            "date": "2012-12-24",
            "value": 55
        }, {
            "date": "2012-12-25",
            "value": 52
        }, {
            "date": "2012-12-26",
            "value": 54
        }, {
            "date": "2012-12-27",
            "value": 50
        }, {
            "date": "2012-12-28",
            "value": 50
        }, {
            "date": "2012-12-29",
            "value": 51
        }, {
            "date": "2012-12-30",
            "value": 52
        }, {
            "date": "2012-12-31",
            "value": 58
        }, {
            "date": "2013-01-01",
            "value": 60
        }, {
            "date": "2013-01-02",
            "value": 67
        }, {
            "date": "2013-01-03",
            "value": 64
        }, {
            "date": "2013-01-04",
            "value": 66
        }, {
            "date": "2013-01-05",
            "value": 60
        }, {
            "date": "2013-01-06",
            "value": 63
        }, {
            "date": "2013-01-07",
            "value": 61
        }, {
            "date": "2013-01-08",
            "value": 60
        }, {
            "date": "2013-01-09",
            "value": 65
        }, {
            "date": "2013-01-10",
            "value": 75
        }, {
            "date": "2013-01-11",
            "value": 77
        }, {
            "date": "2013-01-12",
            "value": 78
        }, {
            "date": "2013-01-13",
            "value": 70
        }, {
            "date": "2013-01-14",
            "value": 70
        }, {
            "date": "2013-01-15",
            "value": 73
        }, {
            "date": "2013-01-16",
            "value": 71
        }, {
            "date": "2013-01-17",
            "value": 74
        }, {
            "date": "2013-01-18",
            "value": 78
        }, {
            "date": "2013-01-19",
            "value": 85
        }, {
            "date": "2013-01-20",
            "value": 82
        }, {
            "date": "2013-01-21",
            "value": 83
        }, {
            "date": "2013-01-22",
            "value": 88
        }, {
            "date": "2013-01-23",
            "value": 85
        }, {
            "date": "2013-01-24",
            "value": 85
        }, {
            "date": "2013-01-25",
            "value": 80
        }, {
            "date": "2013-01-26",
            "value": 87
        }, {
            "date": "2013-01-27",
            "value": 84
        }, {
            "date": "2013-01-28",
            "value": 83
        }, {
            "date": "2013-01-29",
            "value": 84
        }, {
            "date": "2013-01-30",
            "value": 81
        }];

        // Set input format for the dates
        chart.dateFormatter.inputDateFormat = "yyyy-MM-dd";

        // Create axes
        var dateAxis = chart.xAxes.push(new am4charts.DateAxis());
        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());

        // Create series
        var series = chart.series.push(new am4charts.LineSeries());
        series.dataFields.valueY = "value";
        series.dataFields.dateX = "date";
        series.tooltipText = "{value}"
        series.strokeWidth = 2;
        series.minBulletDistance = 15;

        // Drop-shaped tooltips
        series.tooltip.background.cornerRadius = 20;
        series.tooltip.background.strokeOpacity = 0;
        series.tooltip.pointerOrientation = "vertical";
        series.tooltip.label.minWidth = 40;
        series.tooltip.label.minHeight = 40;
        series.tooltip.label.textAlign = "middle";
        series.tooltip.label.textValign = "middle";

        // Make bullets grow on hover
        var bullet = series.bullets.push(new am4charts.CircleBullet());
        bullet.circle.strokeWidth = 2;
        bullet.circle.radius = 4;
        bullet.circle.fill = am4core.color("#fff");

        var bullethover = bullet.states.create("hover");
        bullethover.properties.scale = 1.3;

        // Make a panning cursor
        chart.cursor = new am4charts.XYCursor();
        chart.cursor.behavior = "panXY";
        chart.cursor.xAxis = dateAxis;
        chart.cursor.snapToSeries = series;

        // Create vertical scrollbar and place it before the value axis
        chart.scrollbarY = new am4core.Scrollbar();
        chart.scrollbarY.parent = chart.leftAxesContainer;
        chart.scrollbarY.toBack();

        // Create a horizontal scrollbar with previe and place it underneath the date axis
        chart.scrollbarX = new am4charts.XYChartScrollbar();
        chart.scrollbarX.series.push(series);
        chart.scrollbarX.parent = chart.bottomAxesContainer;

        dateAxis.start = 0.79;
        dateAxis.keepSelection = true;


    }); // end am4core.ready()
}
if (jQuery('#am-linescrollzomm-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-linescrollzomm-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab")];

        // Add data
        chart.data = generateChartData();

        // Create axes
        var dateAxis = chart.xAxes.push(new am4charts.DateAxis());
        dateAxis.renderer.minGridDistance = 50;

        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());

        // Create series
        var series = chart.series.push(new am4charts.LineSeries());
        series.dataFields.valueY = "visits";
        series.dataFields.dateX = "date";
        series.strokeWidth = 2;
        series.minBulletDistance = 10;
        series.tooltipText = "{valueY}";
        series.tooltip.pointerOrientation = "vertical";
        series.tooltip.background.cornerRadius = 20;
        series.tooltip.background.fillOpacity = 0.5;
        series.tooltip.label.padding(12, 12, 12, 12)

        // Add scrollbar
        chart.scrollbarX = new am4charts.XYChartScrollbar();
        chart.scrollbarX.series.push(series);

        // Add cursor
        chart.cursor = new am4charts.XYCursor();
        chart.cursor.xAxis = dateAxis;
        chart.cursor.snapToSeries = series;

        function generateChartData() {
            var chartData = [];
            var firstDate = new Date();
            firstDate.setDate(firstDate.getDate() - 1000);
            var visits = 1200;
            for (var i = 0; i < 500; i++) {
                var newDate = new Date(firstDate);
                newDate.setDate(newDate.getDate() + i);

                visits += Math.round((Math.random() < 0.5 ? 1 : -1) * Math.random() * 10);

                chartData.push({
                    date: newDate,
                    visits: visits
                });
            }
            return chartData;
        }

    });
}

if (jQuery('#am-zoomable-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-zoomable-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#089bab")];

        // Add data
        chart.data = [{
            "date": "2012-07-27",
            "value": 13
        }, {
            "date": "2012-07-28",
            "value": 11
        }, {
            "date": "2012-07-29",
            "value": 15
        }, {
            "date": "2012-07-30",
            "value": 16
        }, {
            "date": "2012-07-31",
            "value": 18
        }, {
            "date": "2012-08-01",
            "value": 13
        }, {
            "date": "2012-08-02",
            "value": 22
        }, {
            "date": "2012-08-03",
            "value": 23
        }, {
            "date": "2012-08-04",
            "value": 20
        }, {
            "date": "2012-08-05",
            "value": 17
        }, {
            "date": "2012-08-06",
            "value": 16
        }, {
            "date": "2012-08-07",
            "value": 18
        }, {
            "date": "2012-08-08",
            "value": 21
        }, {
            "date": "2012-08-09",
            "value": 26
        }, {
            "date": "2012-08-10",
            "value": 24
        }, {
            "date": "2012-08-11",
            "value": 29
        }, {
            "date": "2012-08-12",
            "value": 32
        }, {
            "date": "2012-08-13",
            "value": 18
        }, {
            "date": "2012-08-14",
            "value": 24
        }, {
            "date": "2012-08-15",
            "value": 22
        }, {
            "date": "2012-08-16",
            "value": 18
        }, {
            "date": "2012-08-17",
            "value": 19
        }, {
            "date": "2012-08-18",
            "value": 14
        }, {
            "date": "2012-08-19",
            "value": 15
        }, {
            "date": "2012-08-20",
            "value": 12
        }, {
            "date": "2012-08-21",
            "value": 8
        }, {
            "date": "2012-08-22",
            "value": 9
        }, {
            "date": "2012-08-23",
            "value": 8
        }, {
            "date": "2012-08-24",
            "value": 7
        }, {
            "date": "2012-08-25",
            "value": 5
        }, {
            "date": "2012-08-26",
            "value": 11
        }, {
            "date": "2012-08-27",
            "value": 13
        }, {
            "date": "2012-08-28",
            "value": 18
        }, {
            "date": "2012-08-29",
            "value": 20
        }, {
            "date": "2012-08-30",
            "value": 29
        }, {
            "date": "2012-08-31",
            "value": 33
        }, {
            "date": "2012-09-01",
            "value": 42
        }, {
            "date": "2012-09-02",
            "value": 35
        }, {
            "date": "2012-09-03",
            "value": 31
        }, {
            "date": "2012-09-04",
            "value": 47
        }, {
            "date": "2012-09-05",
            "value": 52
        }, {
            "date": "2012-09-06",
            "value": 46
        }, {
            "date": "2012-09-07",
            "value": 41
        }, {
            "date": "2012-09-08",
            "value": 43
        }, {
            "date": "2012-09-09",
            "value": 40
        }, {
            "date": "2012-09-10",
            "value": 39
        }, {
            "date": "2012-09-11",
            "value": 34
        }, {
            "date": "2012-09-12",
            "value": 29
        }, {
            "date": "2012-09-13",
            "value": 34
        }, {
            "date": "2012-09-14",
            "value": 37
        }, {
            "date": "2012-09-15",
            "value": 42
        }, {
            "date": "2012-09-16",
            "value": 49
        }, {
            "date": "2012-09-17",
            "value": 46
        }, {
            "date": "2012-09-18",
            "value": 47
        }, {
            "date": "2012-09-19",
            "value": 55
        }, {
            "date": "2012-09-20",
            "value": 59
        }, {
            "date": "2012-09-21",
            "value": 58
        }, {
            "date": "2012-09-22",
            "value": 57
        }, {
            "date": "2012-09-23",
            "value": 61
        }, {
            "date": "2012-09-24",
            "value": 59
        }, {
            "date": "2012-09-25",
            "value": 67
        }, {
            "date": "2012-09-26",
            "value": 65
        }, {
            "date": "2012-09-27",
            "value": 61
        }, {
            "date": "2012-09-28",
            "value": 66
        }, {
            "date": "2012-09-29",
            "value": 69
        }, {
            "date": "2012-09-30",
            "value": 71
        }, {
            "date": "2012-10-01",
            "value": 67
        }, {
            "date": "2012-10-02",
            "value": 63
        }, {
            "date": "2012-10-03",
            "value": 46
        }, {
            "date": "2012-10-04",
            "value": 32
        }, {
            "date": "2012-10-05",
            "value": 21
        }, {
            "date": "2012-10-06",
            "value": 18
        }, {
            "date": "2012-10-07",
            "value": 21
        }, {
            "date": "2012-10-08",
            "value": 28
        }, {
            "date": "2012-10-09",
            "value": 27
        }, {
            "date": "2012-10-10",
            "value": 36
        }, {
            "date": "2012-10-11",
            "value": 33
        }, {
            "date": "2012-10-12",
            "value": 31
        }, {
            "date": "2012-10-13",
            "value": 30
        }, {
            "date": "2012-10-14",
            "value": 34
        }, {
            "date": "2012-10-15",
            "value": 38
        }, {
            "date": "2012-10-16",
            "value": 37
        }, {
            "date": "2012-10-17",
            "value": 44
        }, {
            "date": "2012-10-18",
            "value": 49
        }, {
            "date": "2012-10-19",
            "value": 53
        }, {
            "date": "2012-10-20",
            "value": 57
        }, {
            "date": "2012-10-21",
            "value": 60
        }, {
            "date": "2012-10-22",
            "value": 61
        }, {
            "date": "2012-10-23",
            "value": 69
        }, {
            "date": "2012-10-24",
            "value": 67
        }, {
            "date": "2012-10-25",
            "value": 72
        }, {
            "date": "2012-10-26",
            "value": 77
        }, {
            "date": "2012-10-27",
            "value": 75
        }, {
            "date": "2012-10-28",
            "value": 70
        }, {
            "date": "2012-10-29",
            "value": 72
        }, {
            "date": "2012-10-30",
            "value": 70
        }, {
            "date": "2012-10-31",
            "value": 72
        }, {
            "date": "2012-11-01",
            "value": 73
        }, {
            "date": "2012-11-02",
            "value": 67
        }, {
            "date": "2012-11-03",
            "value": 68
        }, {
            "date": "2012-11-04",
            "value": 65
        }, {
            "date": "2012-11-05",
            "value": 71
        }, {
            "date": "2012-11-06",
            "value": 75
        }, {
            "date": "2012-11-07",
            "value": 74
        }, {
            "date": "2012-11-08",
            "value": 71
        }, {
            "date": "2012-11-09",
            "value": 76
        }, {
            "date": "2012-11-10",
            "value": 77
        }, {
            "date": "2012-11-11",
            "value": 81
        }, {
            "date": "2012-11-12",
            "value": 83
        }, {
            "date": "2012-11-13",
            "value": 80
        }, {
            "date": "2012-11-18",
            "value": 80
        }, {
            "date": "2012-11-19",
            "value": 87
        }, {
            "date": "2012-11-20",
            "value": 83
        }, {
            "date": "2012-11-21",
            "value": 85
        }, {
            "date": "2012-11-22",
            "value": 84
        }, {
            "date": "2012-11-23",
            "value": 82
        }, {
            "date": "2012-11-24",
            "value": 73
        }, {
            "date": "2012-11-25",
            "value": 71
        }, {
            "date": "2012-11-26",
            "value": 75
        }, {
            "date": "2012-11-27",
            "value": 79
        }, {
            "date": "2012-11-28",
            "value": 70
        }, {
            "date": "2012-11-29",
            "value": 73
        }, {
            "date": "2012-11-30",
            "value": 61
        }, {
            "date": "2012-12-01",
            "value": 62
        }, {
            "date": "2012-12-02",
            "value": 66
        }, {
            "date": "2012-12-03",
            "value": 65
        }, {
            "date": "2012-12-04",
            "value": 73
        }, {
            "date": "2012-12-05",
            "value": 79
        }, {
            "date": "2012-12-06",
            "value": 78
        }, {
            "date": "2012-12-07",
            "value": 78
        }, {
            "date": "2012-12-08",
            "value": 78
        }, {
            "date": "2012-12-09",
            "value": 74
        }, {
            "date": "2012-12-10",
            "value": 73
        }, {
            "date": "2012-12-11",
            "value": 75
        }, {
            "date": "2012-12-12",
            "value": 70
        }, {
            "date": "2012-12-13",
            "value": 77
        }, {
            "date": "2012-12-14",
            "value": 67
        }, {
            "date": "2012-12-15",
            "value": 62
        }, {
            "date": "2012-12-16",
            "value": 64
        }, {
            "date": "2012-12-17",
            "value": 61
        }, {
            "date": "2012-12-18",
            "value": 59
        }, {
            "date": "2012-12-19",
            "value": 53
        }, {
            "date": "2012-12-20",
            "value": 54
        }, {
            "date": "2012-12-21",
            "value": 56
        }, {
            "date": "2012-12-22",
            "value": 59
        }, {
            "date": "2012-12-23",
            "value": 58
        }, {
            "date": "2012-12-24",
            "value": 55
        }, {
            "date": "2012-12-25",
            "value": 52
        }, {
            "date": "2012-12-26",
            "value": 54
        }, {
            "date": "2012-12-27",
            "value": 50
        }, {
            "date": "2012-12-28",
            "value": 50
        }, {
            "date": "2012-12-29",
            "value": 51
        }, {
            "date": "2012-12-30",
            "value": 52
        }, {
            "date": "2012-12-31",
            "value": 58
        }, {
            "date": "2013-01-01",
            "value": 60
        }, {
            "date": "2013-01-02",
            "value": 67
        }, {
            "date": "2013-01-03",
            "value": 64
        }, {
            "date": "2013-01-04",
            "value": 66
        }, {
            "date": "2013-01-05",
            "value": 60
        }, {
            "date": "2013-01-06",
            "value": 63
        }, {
            "date": "2013-01-07",
            "value": 61
        }, {
            "date": "2013-01-08",
            "value": 60
        }, {
            "date": "2013-01-09",
            "value": 65
        }, {
            "date": "2013-01-10",
            "value": 75
        }, {
            "date": "2013-01-11",
            "value": 77
        }, {
            "date": "2013-01-12",
            "value": 78
        }, {
            "date": "2013-01-13",
            "value": 70
        }, {
            "date": "2013-01-14",
            "value": 70
        }, {
            "date": "2013-01-15",
            "value": 73
        }, {
            "date": "2013-01-16",
            "value": 71
        }, {
            "date": "2013-01-17",
            "value": 74
        }, {
            "date": "2013-01-18",
            "value": 78
        }, {
            "date": "2013-01-19",
            "value": 85
        }, {
            "date": "2013-01-20",
            "value": 82
        }, {
            "date": "2013-01-21",
            "value": 83
        }, {
            "date": "2013-01-22",
            "value": 88
        }, {
            "date": "2013-01-23",
            "value": 85
        }, {
            "date": "2013-01-24",
            "value": 85
        }, {
            "date": "2013-01-25",
            "value": 80
        }, {
            "date": "2013-01-26",
            "value": 87
        }, {
            "date": "2013-01-27",
            "value": 84
        }, {
            "date": "2013-01-28",
            "value": 83
        }, {
            "date": "2013-01-29",
            "value": 84
        }, {
            "date": "2013-01-30",
            "value": 81
        }];

        // Create axes
        var dateAxis = chart.xAxes.push(new am4charts.DateAxis());
        dateAxis.renderer.grid.template.location = 0;
        dateAxis.renderer.minGridDistance = 50;

        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());

        // Create series
        var series = chart.series.push(new am4charts.LineSeries());
        series.dataFields.valueY = "value";
        series.dataFields.dateX = "date";
        series.strokeWidth = 3;
        series.fillOpacity = 0.5;

        // Add vertical scrollbar
        chart.scrollbarY = new am4core.Scrollbar();
        chart.scrollbarY.marginLeft = 0;

        // Add cursor
        chart.cursor = new am4charts.XYCursor();
        chart.cursor.behavior = "zoomY";
        chart.cursor.lineX.disabled = true;

    }); // end am4core.ready()
}
if (jQuery('#am-radar-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        /* Create chart instance */
        var chart = am4core.create("am-radar-chart", am4charts.RadarChart);
        chart.colors.list = [am4core.color("#089bab")];

        /* Add data */
        chart.data = [{
            "country": "Lithuania",
            "litres": 501
        }, {
            "country": "Czechia",
            "litres": 301
        }, {
            "country": "Ireland",
            "litres": 266
        }, {
            "country": "Germany",
            "litres": 165
        }, {
            "country": "Australia",
            "litres": 139
        }, {
            "country": "Austria",
            "litres": 336
        }, {
            "country": "UK",
            "litres": 290
        }, {
            "country": "Belgium",
            "litres": 325
        }, {
            "country": "The Netherlands",
            "litres": 40
        }];

        /* Create axes */
        var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "country";

        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
        valueAxis.renderer.axisFills.template.fill = chart.colors.getIndex(2);
        valueAxis.renderer.axisFills.template.fillOpacity = 0.05;

        /* Create and configure series */
        var series = chart.series.push(new am4charts.RadarSeries());
        series.dataFields.valueY = "litres";
        series.dataFields.categoryX = "country";
        series.name = "Sales";
        series.strokeWidth = 3;

    }); // end am4core.ready()
}
if (jQuery('#am-polar-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        /* Create chart instance */
        var chart = am4core.create("am-polar-chart", am4charts.RadarChart);

        /* Add data */
        chart.data = [{
            "direction": "N",
            "value": 8
        }, {
            "direction": "NE",
            "value": 9
        }, {
            "direction": "E",
            "value": 4.5
        }, {
            "direction": "SE",
            "value": 3.5
        }, {
            "direction": "S",
            "value": 9.2
        }, {
            "direction": "SW",
            "value": 8.4
        }, {
            "direction": "W",
            "value": 11.1
        }, {
            "direction": "NW",
            "value": 10
        }];

        /* Create axes */
        var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "direction";

        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
        //valueAxis.renderer.gridType = "polygons";

        var range = categoryAxis.axisRanges.create();
        range.category = "NW";
        range.endCategory = "NW";
        range.axisFill.fill = am4core.color("#089bab");
        range.axisFill.fillOpacity = 0.3;

        var range2 = categoryAxis.axisRanges.create();
        range2.category = "N";
        range2.endCategory = "N";
        range2.axisFill.fill = am4core.color("#e64141");
        range2.axisFill.fillOpacity = 0.3;

        var range3 = categoryAxis.axisRanges.create();
        range3.category = "SE";
        range3.endCategory = "SW";
        range3.axisFill.fill = am4core.color("#FC9F5B");
        range3.axisFill.fillOpacity = 0.3;
        range3.locations.endCategory = 0;

        /* Create and configure series */
        var series = chart.series.push(new am4charts.RadarSeries());
        series.dataFields.valueY = "value";
        series.dataFields.categoryX = "direction";
        series.name = "Wind direction";
        series.strokeWidth = 3;
        series.fillOpacity = 0.2;

    }); // end am4core.ready()
}

if (jQuery('#am-polarscatter-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        /* Create chart instance */
        var chart = am4core.create("am-polarscatter-chart", am4charts.RadarChart);
        chart.colors.list = [am4core.color("#089bab"), am4core.color("#FC9F5B"), am4core.color("#e64141")];

        /* Add data */
        chart.data = [{
            "country": "Lithuania",
            "litres": 501,
            "units": 250
        }, {
            "country": "Czech Republic",
            "litres": 301,
            "units": 222
        }, {
            "country": "Ireland",
            "litres": 266,
            "units": 179
        }, {
            "country": "Germany",
            "litres": 165,
            "units": 298
        }, {
            "country": "Australia",
            "litres": 139,
            "units": 299
        }];

        /* Create axes */
        var xAxis = chart.xAxes.push(new am4charts.ValueAxis());
        xAxis.renderer.maxLabelPosition = 0.99;

        var yAxis = chart.yAxes.push(new am4charts.ValueAxis());
        yAxis.renderer.labels.template.verticalCenter = "bottom";
        yAxis.renderer.labels.template.horizontalCenter = "right";
        yAxis.renderer.maxLabelPosition = 0.99;
        yAxis.renderer.labels.template.paddingBottom = 1;
        yAxis.renderer.labels.template.paddingRight = 3;

        /* Create and configure series */
        var series1 = chart.series.push(new am4charts.RadarSeries());
        series1.bullets.push(new am4charts.CircleBullet());
        series1.strokeOpacity = 0;
        series1.dataFields.valueX = "x";
        series1.dataFields.valueY = "y";
        series1.name = "Series #1";
        series1.sequencedInterpolation = true;
        series1.sequencedInterpolationDelay = 10;
        series1.data = [{
            "x": 83,
            "y": 5.1
        }, {
            "x": 44,
            "y": 5.8
        }, {
            "x": 76,
            "y": 9
        }, {
            "x": 2,
            "y": 1.4
        }, {
            "x": 100,
            "y": 8.3
        }, {
            "x": 96,
            "y": 1.7
        }, {
            "x": 68,
            "y": 3.9
        }, {
            "x": 0,
            "y": 3
        }, {
            "x": 100,
            "y": 4.1
        }, {
            "x": 16,
            "y": 5.5
        }, {
            "x": 71,
            "y": 6.8
        }, {
            "x": 100,
            "y": 7.9
        }, {
            "x": 35,
            "y": 8
        }, {
            "x": 44,
            "y": 6
        }, {
            "x": 64,
            "y": 0.7
        }, {
            "x": 53,
            "y": 3.3
        }, {
            "x": 92,
            "y": 4.1
        }, {
            "x": 43,
            "y": 7.3
        }, {
            "x": 15,
            "y": 7.5
        }, {
            "x": 43,
            "y": 4.3
        }, {
            "x": 90,
            "y": 9.9
        }];

        var series2 = chart.series.push(new am4charts.RadarSeries());
        series2.bullets.push(new am4charts.CircleBullet());
        series2.strokeOpacity = 0;
        series2.dataFields.valueX = "x";
        series2.dataFields.valueY = "y";
        series2.name = "Series #2";
        series2.sequencedInterpolation = true;
        series2.sequencedInterpolationDelay = 10;
        series2.data = [{
            "x": 178,
            "y": 1.3
        }, {
            "x": 129,
            "y": 3.4
        }, {
            "x": 99,
            "y": 2.4
        }, {
            "x": 80,
            "y": 9.9
        }, {
            "x": 118,
            "y": 9.4
        }, {
            "x": 103,
            "y": 8.7
        }, {
            "x": 91,
            "y": 4.2
        }, {
            "x": 151,
            "y": 1.2
        }, {
            "x": 168,
            "y": 5.2
        }, {
            "x": 168,
            "y": 1.6
        }, {
            "x": 152,
            "y": 1.2
        }, {
            "x": 138,
            "y": 7.7
        }, {
            "x": 107,
            "y": 3.9
        }, {
            "x": 124,
            "y": 0.7
        }, {
            "x": 130,
            "y": 2.6
        }, {
            "x": 86,
            "y": 9.2
        }, {
            "x": 169,
            "y": 7.5
        }, {
            "x": 122,
            "y": 9.9
        }, {
            "x": 100,
            "y": 3.8
        }, {
            "x": 172,
            "y": 4.1
        }, {
            "x": 140,
            "y": 7.3
        }, {
            "x": 161,
            "y": 2.3
        }, {
            "x": 141,
            "y": 0.9
        }];

        var series3 = chart.series.push(new am4charts.RadarSeries());
        series3.bullets.push(new am4charts.CircleBullet());
        series3.strokeOpacity = 0;
        series3.dataFields.valueX = "x";
        series3.dataFields.valueY = "y";
        series3.name = "Series #3";
        series3.sequencedInterpolation = true;
        series3.sequencedInterpolationDelay = 10;
        series3.data = [{
            "x": 419,
            "y": 4.9
        }, {
            "x": 417,
            "y": 5.5
        }, {
            "x": 434,
            "y": 0.1
        }, {
            "x": 344,
            "y": 2.5
        }, {
            "x": 279,
            "y": 7.5
        }, {
            "x": 307,
            "y": 8.4
        }, {
            "x": 279,
            "y": 9
        }, {
            "x": 220,
            "y": 8.4
        }, {
            "x": 201,
            "y": 9.7
        }, {
            "x": 288,
            "y": 1.2
        }, {
            "x": 333,
            "y": 7.4
        }, {
            "x": 308,
            "y": 1.9
        }, {
            "x": 330,
            "y": 8
        }, {
            "x": 408,
            "y": 1.7
        }, {
            "x": 274,
            "y": 0.8
        }, {
            "x": 296,
            "y": 3.1
        }, {
            "x": 279,
            "y": 4.3
        }, {
            "x": 379,
            "y": 5.6
        }, {
            "x": 175,
            "y": 6.8
        }];

        /* Add legend */
        chart.legend = new am4charts.Legend();

        /* Add cursor */
        chart.cursor = new am4charts.RadarCursor();


    }); // end am4core.ready()
}
if (jQuery('#am-3dpie-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        var chart = am4core.create("am-3dpie-chart", am4charts.PieChart3D);
        chart.hiddenState.properties.opacity = 0; // this creates initial fade-in

        chart.legend = new am4charts.Legend();

        chart.data = [{
            country: "Lithuania",
            litres: 501.9,
            fill: "red"
        }, {
            country: "Germany",
            litres: 165.8
        }, {
            country: "Australia",
            litres: 139.9
        }, {
            country: "Austria",
            litres: 128.3
        }, {
            country: "UK",
            litres: 99
        }, {
            country: "Belgium",
            litres: 60
        }];

        var series = chart.series.push(new am4charts.PieSeries3D());
        series.colors.list = [am4core.color("#089bab"), am4core.color("#FC9F5B"), am4core.color("#57de53"),
            am4core.color("#f26361"), am4core.color("#ababab"), am4core.color("#61e2fc")
        ];
        series.dataFields.value = "litres";
        series.dataFields.category = "country";

    }); // end am4core.ready()
}

if (jQuery('#am-layeredcolumn-chart').length) {
    am4core.ready(function() {

        // Themes begin
        am4core.useTheme(am4themes_animated);
        // Themes end

        // Create chart instance
        var chart = am4core.create("am-layeredcolumn-chart", am4charts.XYChart);
        chart.colors.list = [am4core.color("#FC9F5B"), am4core.color("#089bab")];

        // Add percent sign to all numbers
        chart.numberFormatter.numberFormat = "#.#'%'";

        // Add data
        chart.data = [{
            "country": "USA",
            "year2004": 3.5,
            "year2005": 4.2
        }, {
            "country": "UK",
            "year2004": 1.7,
            "year2005": 3.1
        }, {
            "country": "Canada",
            "year2004": 2.8,
            "year2005": 2.9
        }, {
            "country": "Japan",
            "year2004": 2.6,
            "year2005": 2.3
        }, {
            "country": "France",
            "year2004": 1.4,
            "year2005": 2.1
        }, {
            "country": "Brazil",
            "year2004": 2.6,
            "year2005": 4.9
        }];

        // Create axes
        var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
        categoryAxis.dataFields.category = "country";
        categoryAxis.renderer.grid.template.location = 0;
        categoryAxis.renderer.minGridDistance = 30;

        var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
        valueAxis.title.text = "GDP growth rate";
        valueAxis.title.fontWeight = 800;

        // Create series
        var series = chart.series.push(new am4charts.ColumnSeries());
        series.dataFields.valueY = "year2004";
        series.dataFields.categoryX = "country";
        series.clustered = false;
        series.tooltipText = "GDP grow in {categoryX} (2004): [bold]{valueY}[/]";

        var series2 = chart.series.push(new am4charts.ColumnSeries());
        series2.dataFields.valueY = "year2005";
        series2.dataFields.categoryX = "country";
        series2.clustered = false;
        series2.columns.template.width = am4core.percent(50);
        series2.tooltipText = "GDP grow in {categoryX} (2005): [bold]{valueY}[/]";

        chart.cursor = new am4charts.XYCursor();
        chart.cursor.lineX.disabled = true;
        chart.cursor.lineY.disabled = true;

    }); // end am4core.ready()
}

if (jQuery('#morris-line-chart').length) {
    new Morris.Line({
        // ID of the element in which to draw the chart.
        element: 'morris-line-chart',
        // Chart data records -- each entry in this array corresponds to a point on
        // the chart.
        data: [{
            year: '2008',
            value: 20
        }, {
            year: '2009',
            value: 10
        }, {
            year: '2010',
            value: 5
        }, {
            year: '2011',
            value: 5
        }, {
            year: '2012',
            value: 20
        }],
        // The name of the data record attribute that contains x-values.
        xkey: 'year',
        // A list of names of data record attributes that contain y-values.
        ykeys: ['value'],
        // Labels for the ykeys -- will be displayed when you hover over the
        // chart.
        labels: ['Value'],
        lineColors: ['#089bab']
    });
}

if (jQuery('#morris-bar-chart').length) {
    Morris.Bar({
        element: 'morris-bar-chart',
        data: [{
            x: '2011 Q1',
            y: 3,
            z: 2,
            a: 3
        }, {
            x: '2011 Q2',
            y: 2,
            z: null,
            a: 1
        }, {
            x: '2011 Q3',
            y: 0,
            z: 2,
            a: 4
        }, {
            x: '2011 Q4',
            y: 2,
            z: 4,
            a: 3
        }],
        xkey: 'x',
        ykeys: ['y', 'z', 'a'],
        labels: ['Y', 'Z', 'A'],
        barColors: ['#089bab', '#FC9F5B ', '#e64141'],
        hoverCallback: function(index, options, content, row) {
            return '';
        }
    }).on('click', function(i, row) {
        console.log(i, row);
    });
}

if (jQuery('#morris-area-chart').length) {
    var area = new Morris.Area({
        element: 'morris-area-chart',
        resize: true,
        data: [{
            y: '2011 Q1',
            item1: 2666,
            item2: 2666
        }, {
            y: '2011 Q2',
            item1: 2778,
            item2: 2294
        }, {
            y: '2011 Q3',
            item1: 4912,
            item2: 1969
        }, {
            y: '2011 Q4',
            item1: 3767,
            item2: 3597
        }, {
            y: '2012 Q1',
            item1: 6810,
            item2: 1914
        }, {
            y: '2012 Q2',
            item1: 5670,
            item2: 4293
        }, {
            y: '2012 Q3',
            item1: 4820,
            item2: 3795
        }, {
            y: '2012 Q4',
            item1: 15073,
            item2: 5967
        }, {
            y: '2013 Q1',
            item1: 10687,
            item2: 4460
        }, {
            y: '2013 Q2',
            item1: 8432,
            item2: 5713
        }],
        xkey: 'y',
        ykeys: ['item1', 'item2'],
        labels: ['Item 1', 'Item 2'],
        lineColors: ['#089bab', '#FC9F5B '],
        hoverCallback: function(index, options, content, row) {
            return '';
        }
    });
}

if (jQuery('#morris-donut-chart').length) {
    var donut = new Morris.Donut({
        element: 'morris-donut-chart',
        resize: true,
        colors: ["#089bab", "#FC9F5B", "#f26361"],
        data: [{
            label: "Download Sales",
            value: 12
        }, {
            label: "In-Store Sales",
            value: 30
        }, {
            label: "Mail-Order Sales",
            value: 20
        }],
        hideHover: 'auto'
    });
}

// High Chart
if (jQuery('#high-basicline-chart').length) {
    Highcharts.chart('high-basicline-chart', {

        title: {
            text: 'Solar Employment Growth by Sector, 2010-2016'
        },

        subtitle: {
            text: 'Source: thesolarfoundation.com'
        },

        yAxis: {
            title: {
                text: 'Number of Employees'
            }
        },
        legend: {
            layout: 'vertical',
            align: 'right',
            verticalAlign: 'middle'
        },

        plotOptions: {
            series: {
                label: {
                    connectorAllowed: false
                },
                pointStart: 2010
            }
        },

        series: [{
            name: 'Installation',
            data: [43934, 52503, 57177, 69658, 97031, 119931, 137133, 154175],
            color: '#089bab'
        }, {
            name: 'Manufacturing',
            data: [24916, 24064, 29742, 29851, 32490, 30282, 38121, 40434],
            color: '#e64141'
        }, {
            name: 'Sales & Distribution',
            data: [11744, 17722, 16005, 19771, 20185, 24377, 32147, 39387],
            color: '#28a745'
        }, {
            name: 'Project Development',
            data: [null, null, 7988, 12169, 15112, 22452, 34400, 34227],
            color: '#ffc107'
        }, {
            name: 'Other',
            data: [12908, 5948, 8105, 11248, 8989, 11816, 18274, 18111],
            color: '#17a2b8'
        }],

        responsive: {
            rules: [{
                condition: {
                    maxWidth: 500
                },
                chartOptions: {
                    legend: {
                        layout: 'horizontal',
                        align: 'center',
                        verticalAlign: 'bottom'
                    }
                }
            }]
        }

    });
}

if (jQuery('#high-area-chart').length) {
    Highcharts.chart('high-area-chart', {
        chart: {
            type: 'area'
        },
        accessibility: {
            description: 'Image description: An area chart compares the nuclear stockpiles of the USA and the USSR/Russia between 1945 and 2017. The number of nuclear weapons is plotted on the Y-axis and the years on the X-axis. The chart is interactive, and the year-on-year stockpile levels can be traced for each country. The US has a stockpile of 6 nuclear weapons at the dawn of the nuclear age in 1945. This number has gradually increased to 369 by 1950 when the USSR enters the arms race with 6 weapons. At this point, the US starts to rapidly build its stockpile culminating in 32,040 warheads by 1966 compared to the USSR’s 7,089. From this peak in 1966, the US stockpile gradually decreases as the USSR’s stockpile expands. By 1978 the USSR has closed the nuclear gap at 25,393. The USSR stockpile continues to grow until it reaches a peak of 45,000 in 1986 compared to the US arsenal of 24,401. From 1986, the nuclear stockpiles of both countries start to fall. By 2000, the numbers have fallen to 10,577 and 21,000 for the US and Russia, respectively. The decreases continue until 2017 at which point the US holds 4,018 weapons compared to Russia’s 4,500.'
        },
        title: {
            text: 'US and USSR nuclear stockpiles'
        },
        subtitle: {
            text: 'Sources: <a href="https://thebulletin.org/2006/july/global-nuclear-stockpiles-1945-2006">' +
                'thebulletin.org</a> &amp; <a href="https://www.armscontrol.org/factsheets/Nuclearweaponswhohaswhat">' +
                'armscontrol.org</a>'
        },
        xAxis: {
            allowDecimals: false,
            labels: {
                formatter: function() {
                    return this.value; // clean, unformatted number for year
                }
            },
            accessibility: {
                rangeDescription: 'Range: 1940 to 2017.'
            }
        },
        yAxis: {
            title: {
                text: 'Nuclear weapon states'
            },
            labels: {
                formatter: function() {
                    return this.value / 1000 + 'k';
                }
            }
        },
        tooltip: {
            pointFormat: '{series.name} had stockpiled <b>{point.y:,.0f}</b><br/>warheads in {point.x}'
        },
        plotOptions: {
            area: {
                pointStart: 1940,
                marker: {
                    enabled: false,
                    symbol: 'circle',
                    radius: 2,
                    states: {
                        hover: {
                            enabled: true
                        }
                    }
                }
            }
        },
        series: [{
            name: 'USA',
            data: [
                null, null, null, null, null, 6, 11, 32, 110, 235,
                369, 640, 1005, 1436, 2063, 3057, 4618, 6444, 9822, 15468,
                20434, 24126, 27387, 29459, 31056, 31982, 32040, 31233, 29224, 27342,
                26662, 26956, 27912, 28999, 28965, 27826, 25579, 25722, 24826, 24605,
                24304, 23464, 23708, 24099, 24357, 24237, 24401, 24344, 23586, 22380,
                21004, 17287, 14747, 13076, 12555, 12144, 11009, 10950, 10871, 10824,
                10577, 10527, 10475, 10421, 10358, 10295, 10104, 9914, 9620, 9326,
                5113, 5113, 4954, 4804, 4761, 4717, 4368, 4018
            ],
            color: '#089bab'
        }, {
            name: 'USSR/Russia',
            data: [null, null, null, null, null, null, null, null, null, null,
                5, 25, 50, 120, 150, 200, 426, 660, 869, 1060,
                1605, 2471, 3322, 4238, 5221, 6129, 7089, 8339, 9399, 10538,
                11643, 13092, 14478, 15915, 17385, 19055, 21205, 23044, 25393, 27935,
                30062, 32049, 33952, 35804, 37431, 39197, 45000, 43000, 41000, 39000,
                37000, 35000, 33000, 31000, 29000, 27000, 25000, 24000, 23000, 22000,
                21000, 20000, 19000, 18000, 18000, 17000, 16000, 15537, 14162, 12787,
                12600, 11400, 5500, 4512, 4502, 4502, 4500, 4500
            ],
            color: '#FC9F5B'
        }]
    });
}

if (jQuery('#high-columnndbar-chart').length) {
    Highcharts.chart('high-columnndbar-chart', {
        chart: {
            type: 'bar'
        },
        title: {
            text: 'Historic World Population by Region'
        },
        subtitle: {
            text: 'Source: <a href="https://en.wikipedia.org/wiki/World_population">Wikipedia.org</a>'
        },
        xAxis: {
            categories: ['Africa', 'America', 'Asia', 'Europe', 'Oceania'],
            title: {
                text: null
            }
        },
        yAxis: {
            min: 0,
            title: {
                text: 'Population (millions)',
                align: 'high'
            },
            labels: {
                overflow: 'justify'
            }
        },
        tooltip: {
            valueSuffix: ' millions'
        },
        plotOptions: {
            bar: {
                dataLabels: {
                    enabled: true
                }
            }
        },
        legend: {
            layout: 'vertical',
            align: 'right',
            verticalAlign: 'top',
            x: -40,
            y: 80,
            floating: true,
            borderWidth: 1,
            backgroundColor: Highcharts.defaultOptions.legend.backgroundColor || '#FFFFFF',
            shadow: true
        },
        credits: {
            enabled: false
        },
        series: [{
            name: 'Year 1900',
            data: [133, 156, 947, 408, 6],
            color: '#089bab'
        }, {
            name: 'Year 2000',
            data: [814, 841, 3714, 727, 31],
            color: '#e64141'
        }, {
            name: 'Year 2016',
            data: [1216, 1001, 4436, 738, 40],
            color: '#FC9F5B'
        }]
    });
}

if (jQuery('#high-pie-chart').length) {
    Highcharts.chart('high-pie-chart', {
        chart: {
            plotBackgroundColor: null,
            plotBorderWidth: null,
            plotShadow: false,
            type: 'pie'
        },
        colorAxis: {},
        title: {
            text: 'Browser market shares in January, 2018'
        },
        tooltip: {
            pointFormat: '{series.name}: <b>{point.percentage:.1f}%</b>'
        },
        plotOptions: {
            pie: {
                allowPointSelect: true,
                cursor: 'pointer',
                dataLabels: {
                    enabled: true,
                    format: '<b>{point.name}</b>: {point.percentage:.1f} %'
                }
            }
        },
        series: [{
            name: 'Brands',
            colorByPoint: true,
            data: [{
                name: 'Chrome',
                y: 61.41,
                sliced: true,
                selected: true,
                color: '#089bab'
            }, {
                name: 'Internet Explorer',
                y: 11.84,
                color: '#FC9F5B'
            }, {
                name: 'Firefox',
                y: 10.85,
                color: '#e64141'
            }, {
                name: 'Edge',
                y: 4.67,
                color: '#ffd400'
            }, {
                name: 'Safari',
                y: 4.18,
                color: '#00d0ff'
            }, {
                name: 'Sogou Explorer',
                y: 1.64,
                color: '#374948'
            }, {
                name: 'Opera',
                y: 1.6,
                color: '#E9EDF4'
            }, {
                name: 'QQ',
                y: 1.2,
                color: '#777D74'
            }, {
                name: 'Other',
                y: 2.61,
            }]
        }]
    });
}
if (jQuery('#high-scatterplot-chart').length) {
    Highcharts.chart('high-scatterplot-chart', {
        chart: {
            type: 'scatter',
            zoomType: 'xy'
        },
        accessibility: {
            description: 'A scatter plot compares the height and weight of 507 individuals by gender. Height in centimeters is plotted on the X-axis and weight in kilograms is plotted on the Y-axis. The chart is interactive, and each data point can be hovered over to expose the height and weight data for each individual. The scatter plot is fairly evenly divided by gender with females dominating the left-hand side of the chart and males dominating the right-hand side. The height data for females ranges from 147.2 to 182.9 centimeters with the greatest concentration between 160 and 165 centimeters. The weight data for females ranges from 42 to 105.2 kilograms with the greatest concentration at around 60 kilograms. The height data for males ranges from 157.2 to 198.1 centimeters with the greatest concentration between 175 and 180 centimeters. The weight data for males ranges from 53.9 to 116.4 kilograms with the greatest concentration at around 80 kilograms.'
        },
        title: {
            text: 'Height Versus Weight of 507 Individuals by Gender'
        },
        subtitle: {
            text: 'Source: Heinz  2003'
        },
        xAxis: {
            title: {
                enabled: true,
                text: 'Height (cm)'
            },
            startOnTick: true,
            endOnTick: true,
            showLastLabel: true
        },
        yAxis: {
            title: {
                text: 'Weight (kg)'
            }
        },
        legend: {
            layout: 'vertical',
            align: 'left',
            verticalAlign: 'top',
            x: 100,
            y: 70,
            floating: true,
            backgroundColor: Highcharts.defaultOptions.chart.backgroundColor,
            borderWidth: 1
        },
        plotOptions: {
            scatter: {
                marker: {
                    radius: 5,
                    states: {
                        hover: {
                            enabled: true,
                            lineColor: 'rgb(100,100,100)'
                        }
                    }
                },
                states: {
                    hover: {
                        marker: {
                            enabled: false
                        }
                    }
                },
                tooltip: {
                    headerFormat: '<b>{series.name}</b><br>',
                    pointFormat: '{point.x} cm, {point.y} kg'
                }
            }
        },
        series: [{
            name: 'Female',
            color: 'rgba(223, 83, 83, .5)',
            data: [
                [161.2, 51.6],
                [167.5, 59.0],
                [159.5, 49.2],
                [157.0, 63.0],
                [155.8, 53.6],
                [170.0, 59.0],
                [159.1, 47.6],
                [166.0, 69.8],
                [176.2, 66.8],
                [160.2, 75.2],
                [172.7, 62.0],
                [155.0, 49.2],
                [156.5, 67.2],
                [164.0, 53.8],
                [160.9, 54.4]
            ],
            color: '#089bab'

        }, {
            name: 'Male',
            color: 'rgba(119, 152, 191, .5)',
            data: [
                [174.0, 65.6],
                [175.3, 71.8],
                [193.5, 80.7],
                [186.5, 72.6],
                [187.2, 78.8],
                [181.5, 74.8],
                [184.0, 86.4],
                [184.5, 78.4],
                [175.0, 62.0],
                [184.0, 81.6],
                [180.1, 93.0],
                [175.5, 80.9],
                [180.6, 72.7],
                [184.4, 68.0],
                [175.5, 70.9],
                [180.3, 83.2],
                [180.3, 83.2]
            ],
            color: '#FC9F5B'
        }]
    });
}

if (jQuery('#high-linendcolumn-chart').length) {
    Highcharts.chart('high-linendcolumn-chart', {
        chart: {
            zoomType: 'xy'
        },
        title: {
            text: 'Average Monthly Temperature and Rainfall in Tokyo'
        },
        subtitle: {
            text: 'Source: WorldClimate.com'
        },
        xAxis: [{
            categories: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
                'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'
            ],
            crosshair: true
        }],
        yAxis: [{ // Primary yAxis
            labels: {
                format: '{value}°C',
                style: {
                    color: Highcharts.getOptions().colors[1]
                }
            },
            title: {
                text: 'Temperature',
                style: {
                    color: Highcharts.getOptions().colors[1]
                }
            }
        }, { // Secondary yAxis
            title: {
                text: 'Rainfall',
                style: {
                    color: Highcharts.getOptions().colors[0]
                }
            },
            labels: {
                format: '{value} mm',
                style: {
                    color: Highcharts.getOptions().colors[0]
                }
            },
            opposite: true
        }],
        tooltip: {
            shared: true
        },
        legend: {
            layout: 'vertical',
            align: 'left',
            x: 120,
            verticalAlign: 'top',
            y: 100,
            floating: true,
            backgroundColor: Highcharts.defaultOptions.legend.backgroundColor || // theme
                'rgba(255,255,255,0.25)'
        },
        series: [{
            name: 'Rainfall',
            type: 'column',
            yAxis: 1,
            data: [49.9, 71.5, 106.4, 129.2, 144.0, 176.0, 135.6, 148.5, 216.4, 194.1, 95.6, 54.4],
            color: '#089bab',
            tooltip: {
                valueSuffix: ' mm'
            }

        }, {
            name: 'Temperature',
            type: 'spline',
            data: [7.0, 6.9, 9.5, 14.5, 18.2, 21.5, 25.2, 26.5, 23.3, 18.3, 13.9, 9.6],
            color: '#FC9F5B',
            tooltip: {
                valueSuffix: '°C'
            }
        }]
    });
}

if (jQuery('#high-dynamic-chart').length) {
    Highcharts.chart('high-dynamic-chart', {
        chart: {
            type: 'spline',
            animation: Highcharts.svg, // don't animate in old IE
            marginRight: 10,
            events: {
                load: function() {

                    // set up the updating of the chart each second
                    var series = this.series[0];
                    setInterval(function() {
                        var x = (new Date()).getTime(), // current time
                            y = Math.random();
                        series.addPoint([x, y], true, true);
                    }, 1000);
                }
            }
        },

        time: {
            useUTC: false
        },

        title: {
            text: 'Live random data'
        },

        accessibility: {
            announceNewData: {
                enabled: true,
                minAnnounceInterval: 15000,
                announcementFormatter: function(allSeries, newSeries, newPoint) {
                    if (newPoint) {
                        return 'New point added. Value: ' + newPoint.y;
                    }
                    return false;
                }
            }
        },

        xAxis: {
            type: 'datetime',
            tickPixelInterval: 150
        },

        yAxis: {
            title: {
                text: 'Value'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },

        tooltip: {
            headerFormat: '<b>{series.name}</b><br/>',
            pointFormat: '{point.x:%Y-%m-%d %H:%M:%S}<br/>{point.y:.2f}'
        },

        legend: {
            enabled: false
        },

        exporting: {
            enabled: false
        },

        series: [{
            name: 'Random data',
            color: '#089bab',
            data: (function() {
                // generate an array of random data
                var data = [],
                    time = (new Date()).getTime(),
                    i;

                for (i = -19; i <= 0; i += 1) {
                    data.push({
                        x: time + i * 1000,
                        y: Math.random()
                    });
                }
                return data;
            }())
        }]
    });
}

if (jQuery('#high-3d-chart').length) {
    var chart = new Highcharts.Chart({
        chart: {
            renderTo: 'high-3d-chart',
            type: 'column',
            options3d: {
                enabled: true,
                alpha: 15,
                beta: 15,
                depth: 50,
                viewDistance: 25
            }
        },
        title: {
            text: 'Chart rotation demo'
        },
        subtitle: {
            text: 'Test options by dragging the sliders below'
        },
        plotOptions: {
            column: {
                depth: 25
            }
        },
        series: [{
            data: [29.9, 71.5, 106.4, 129.2, 144.0, 176.0, 135.6, 148.5, 216.4, 194.1, 95.6, 54.4],
            color: '#089bab'
        }]
    });

    function showValues() {
        $('#alpha-value').html(chart.options.chart.options3d.alpha);
        $('#beta-value').html(chart.options.chart.options3d.beta);
        $('#depth-value').html(chart.options.chart.options3d.depth);
    }

    // Activate the sliders
    $('#sliders input').on('input change', function() {
        chart.options.chart.options3d[this.id] = parseFloat(this.value);
        showValues();
        chart.redraw(false);
    });

    showValues();
}

if (jQuery('#high-gauges-chart').length) {
    Highcharts.chart('high-gauges-chart', {

            chart: {
                type: 'gauge',
                plotBackgroundColor: null,
                plotBackgroundImage: null,
                plotBorderWidth: 0,
                plotShadow: false
            },

            title: {
                text: 'Speedometer'
            },

            pane: {
                startAngle: -150,
                endAngle: 150,
                background: [{
                    backgroundColor: {
                        linearGradient: {
                            x1: 0,
                            y1: 0,
                            x2: 0,
                            y2: 1
                        },
                        stops: [
                            [0, '#FFF'],
                            [1, '#333']
                        ]
                    },
                    borderWidth: 0,
                    outerRadius: '109%'
                }, {
                    backgroundColor: {
                        linearGradient: {
                            x1: 0,
                            y1: 0,
                            x2: 0,
                            y2: 1
                        },
                        stops: [
                            [0, '#333'],
                            [1, '#FFF']
                        ]
                    },
                    borderWidth: 1,
                    outerRadius: '107%'
                }, {

                }, {
                    backgroundColor: '#DDD',
                    borderWidth: 0,
                    outerRadius: '105%',
                    innerRadius: '103%'
                }]
            },


            yAxis: {
                min: 0,
                max: 200,

                minorTickInterval: 'auto',
                minorTickWidth: 1,
                minorTickLength: 10,
                minorTickPosition: 'inside',
                minorTickColor: '#666',

                tickPixelInterval: 30,
                tickWidth: 2,
                tickPosition: 'inside',
                tickLength: 10,
                tickColor: '#666',
                labels: {
                    step: 2,
                    rotation: 'auto'
                },
                title: {
                    text: 'km/h'
                },
                plotBands: [{
                    from: 0,
                    to: 120,
                    color: '#55BF3B'
                }, {
                    from: 120,
                    to: 160,
                    color: '#DDDF0D'
                }, {
                    from: 160,
                    to: 200,
                    color: '#DF5353'
                }]
            },

            series: [{
                name: 'Speed',
                data: [80],
                tooltip: {
                    valueSuffix: ' km/h'
                }
            }]

        },

        function(chart) {
            if (!chart.renderer.forExport) {
                setInterval(function() {
                    var point = chart.series[0].points[0],
                        newVal,
                        inc = Math.round((Math.random() - 0.5) * 20);

                    newVal = point.y + inc;
                    if (newVal < 0 || newVal > 200) {
                        newVal = point.y - inc;
                    }

                    point.update(newVal);

                }, 3000);
            }
        });
}

if (jQuery('#high-barwithnagative-chart').length) {
    var categories = [
        '0-4', '5-9', '10-14', '15-19',
        '20-24', '25-29', '30-34', '35-39', '40-44',
        '45-49', '50-54', '55-59', '60-64', '65-69',
        '70-74', '75-79', '80-84', '85-89', '90-94',
        '95-99', '100 + '
    ];

    Highcharts.chart('high-barwithnagative-chart', {
        chart: {
            type: 'bar'
        },
        title: {
            text: 'Population pyramid for Germany, 2018'
        },
        subtitle: {
            text: 'Source: <a href="http://populationpyramid.net/germany/2018/">Population Pyramids of the World from 1950 to 2100</a>'
        },
        accessibility: {
            point: {
                descriptionFormatter: function(point) {
                    var index = point.index + 1,
                        category = point.category,
                        val = Math.abs(point.y),
                        series = point.series.name;

                    return index + ', Age ' + category + ', ' + val + '%. ' + series + '.';
                }
            }
        },
        xAxis: [{
            categories: categories,
            reversed: false,
            labels: {
                step: 1
            },
            accessibility: {
                description: 'Age (male)'
            }
        }, {
            opposite: true,
            reversed: false,
            categories: categories,
            linkedTo: 0,
            labels: {
                step: 1
            },
            accessibility: {
                description: 'Age (female)'
            }
        }],
        yAxis: {
            title: {
                text: null
            },
            labels: {
                formatter: function() {
                    return Math.abs(this.value) + '%';
                }
            },
            accessibility: {
                description: 'Percentage population',
                rangeDescription: 'Range: 0 to 5%'
            }
        },

        plotOptions: {
            series: {
                stacking: 'normal'
            }
        },

        tooltip: {
            formatter: function() {
                return '<b>' + this.series.name + ', age ' + this.point.category + '</b><br/>' +
                    'Population: ' + Highcharts.numberFormat(Math.abs(this.point.y), 1) + '%';
            }
        },

        series: [{
            name: 'Male',
            data: [-2.2, -2.1, -2.2, -2.4, -2.7, -3.0, -3.3, -3.2, -2.9, -3.5, -4.4, -4.1, -0.0],
            color: '#FC9F5B'
        }, {
            name: 'Female',
            data: [
                2.1, 2.0, 2.1, 2.3, 2.6,
                2.9, 3.2, 3.1, 2.9, 3.4,
                0.0
            ],
            color: '#089bab'
        }]
    });
}

if (jQuery('#home-chart-1').length) {
    var options = {
        series: [{
            name: 'series1',
            data: [31, 40, 28, 51, 42, 109, 100]
        }, {
            name: 'series2',
            data: [11, 32, 45, 32, 34, 52, 41]
        }],
        colors: ['#089bab', '#e64141'],
        chart: {
            height: 350,
            type: 'area'
        },
        dataLabels: {
            enabled: false
        },
        stroke: {
            curve: 'smooth'
        },
        xaxis: {
            type: 'datetime',
            categories: ["2018-09-19T00:00:00.000Z", "2018-09-19T01:30:00.000Z", "2018-09-19T02:30:00.000Z", "2018-09-19T03:30:00.000Z", "2018-09-19T04:30:00.000Z", "2018-09-19T05:30:00.000Z", "2018-09-19T06:30:00.000Z"]
        },
        tooltip: {
            x: {
                format: 'dd/MM/yy HH:mm'
            },
        },
    };

    var chart = new ApexCharts(document.querySelector("#home-chart-1"), options);
    chart.render();
}

if (jQuery('#home-pie-chart').length) {
    var options = {
        series: [44, 55],
        chart: {
            type: 'donut',
        },
        colors: ['#089bab', '#e64141'],
        responsive: [{
            breakpoint: 480,
            options: {
                chart: {
                    width: 200
                },
                legend: {
                    position: 'bottom'
                }
            }
        }]
    };

    var chart = new ApexCharts(document.querySelector("#home-pie-chart"), options);
    chart.render();
}
if (jQuery('#chart-task').length) {
    am4core.ready(function() {


        am4core.useTheme(am4themes_animated);


        var chart = am4core.create("chart-task", am4charts.PieChart);
        chart.hiddenState.properties.opacity = 0;

        chart.data = [{
            country: "Team-A",
            value: 401
        }, {
            country: "Team-B",
            value: 300
        }, {
            country: "Team-C",
            value: 200
        }, {
            country: "Team-D",
            value: 165
        }, {
            country: "Team-E",
            value: 139
        }, {
            country: "Team-f",
            value: 128
        }];
        chart.radius = am4core.percent(70);
        chart.innerRadius = am4core.percent(40);
        chart.startAngle = 180;
        chart.endAngle = 360;

        var series = chart.series.push(new am4charts.PieSeries());
        series.colors.list = [am4core.color("#089bab"), am4core.color("#c25a72"), am4core.color("#885e72"),
            am4core.color("#726071"), am4core.color("#5a6171"), am4core.color("#506271")
        ];
        series.dataFields.value = "value";
        series.dataFields.category = "country";

        series.slices.template.cornerRadius = 0;
        series.slices.template.innerCornerRadius = 0;
        series.slices.template.draggable = true;
        series.slices.template.inert = true;
        series.alignLabels = false;

        series.hiddenState.properties.startAngle = 90;
        series.hiddenState.properties.endAngle = 90;

        chart.legend = new am4charts.Legend();

    });
}
if (jQuery('#chart-home3-01').length) {
    var options = {
        series: [{
            name: 'Series 1',
            data: [80, 50, 30, 40, 100, 20],
        }, {
            name: 'Series 2',
            data: [20, 30, 40, 80, 20, 80],
        }, {
            name: 'Series 3',
            data: [44, 76, 78, 13, 43, 10],
        }],
        colors: ['#089bab', '#e64141', '#777D74'],
        chart: {
            height: 350,
            type: 'radar',
            dropShadow: {
                enabled: true,
                blur: 1,
                left: 1,
                top: 1
            }
        },
        title: {
            text: 'Radar Chart - Multi Series'
        },
        stroke: {
            width: 0
        },
        fill: {
            opacity: 0.4
        },
        markers: {
            size: 0
        },
        xaxis: {
            categories: ['2011', '2012', '2013', '2014', '2015', '2016']
        }
    };

    var chart = new ApexCharts(document.querySelector("#chart-home3-01"), options);
    chart.render();
}
if (jQuery("#home-chart-03").length) {
    am4core.ready(function() {

    // Themes begin
    am4core.useTheme(am4themes_animated);
    // Themes end

    var chart = am4core.create("home-chart-03", am4charts.PieChart);
    chart.hiddenState.properties.opacity = 0; 

    chart.data = [
      {
        country: "USA",
        value: 401
      },
      {
        country: "India",
        value: 300
      },
      {
        country: "Australia",
        value: 200
      },
      {
        country: "Brazil",
        value: 100
      }
    ];
    chart.radius = am4core.percent(70);
    chart.innerRadius = am4core.percent(40);
    chart.startAngle = 180;
    chart.endAngle = 360;  

    var series = chart.series.push(new am4charts.PieSeries());
    series.dataFields.value = "value";
    series.dataFields.category = "country";
    series.colors.list = [am4core.color("#089bab"), am4core.color("#2ca5b2"), am4core.color("#faa264"),
        am4core.color("#fcb07a")];

    series.slices.template.cornerRadius = 0;
    series.slices.template.innerCornerRadius = 0;
    series.slices.template.draggable = true;
    series.slices.template.inert = true;
    series.alignLabels = false;

    series.hiddenState.properties.startAngle = 90;
    series.hiddenState.properties.endAngle = 90;

    chart.legend = new am4charts.Legend();

    }); 
}
 jQuery("#home-chart-05").length && am4core.ready(function() {
    var options = {
          series: [{
          name: 'PRODUCT A',
          data: [44, 55, 41, 67, 22, 43]
        }, {
          name: 'PRODUCT B',
          data: [13, 23, 20, 8, 13, 27]
        }, {
          name: 'PRODUCT C',
          data: [11, 17, 15, 15, 21, 14]
        }],
        colors: ['#089bab','#FC9F5B','#5bc5d1'],
          chart: {
          type: 'bar',
          height: 350,
          stacked: true,
          toolbar: {
            show: true
          },
          zoom: {
            enabled: true
          }
        },
        responsive: [{
          breakpoint: 480,
          options: {
            legend: {
              position: 'bottom',
              offsetX: -10,
              offsetY: 0
            }
          }
        }],
        plotOptions: {
          bar: {
            horizontal: false,
          },
        },
        xaxis: {
          type: 'datetime',
          categories: ['01/01/2011 GMT', '01/02/2011 GMT', '01/03/2011 GMT', '01/04/2011 GMT',
            '01/05/2011 GMT', '01/06/2011 GMT'
          ],
        },
        legend: {
          position: 'right',
          offsetY: 40
        },
        fill: {
          opacity: 1
        }
        };

        var chart = new ApexCharts(document.querySelector("#home-chart-05"), options);
        chart.render();
})
 jQuery("#home-chart-06").length && am4core.ready(function() {
    var options = {
          series: [{
          name: 'series1',
          data: [31, 40, 28, 51, 42, 109, 100]
        }],
          chart: {
          height: 350,
          type: 'area'
        },
        colors: ["#089bab"],
        dataLabels: {
          enabled: false
        },
        stroke: {
          curve: 'smooth'
        },
        xaxis: {
          type: 'datetime',
          categories: ["2018-09-19T00:00:00.000Z", "2018-09-19T01:30:00.000Z", "2018-09-19T02:30:00.000Z", "2018-09-19T03:30:00.000Z", "2018-09-19T04:30:00.000Z", "2018-09-19T05:30:00.000Z", "2018-09-19T06:30:00.000Z"]
        },
        tooltip: {
          x: {
            format: 'dd/MM/yy HH:mm'
          },
        },
        };

        var chart = new ApexCharts(document.querySelector("#home-chart-06"), options);
        chart.render();
});
  jQuery("#home-servey-chart").length && am4core.ready(function() {
      var options = {
          series: [{
          name: 'Cash Flow',
          data: [1.45, 5.42, 5.9, -0.42, -12.6, -18.1, -18.2, -14.16, -11.1, -6.09, 0.34, 3.88, 13.07,
            5.8, 2, 7.37, 8.1, 13.57, 15.75, 17.1, 19.8, -27.03, -54.4, -47.2, -43.3, -18.6, -
            48.6, -41.1, -39.6, -37.6, -29.4, -21.4, -2.4
          ]
        }],
          chart: {
          type: 'bar',
          height: 350
        },
        plotOptions: {
          bar: {
            colors: {
              ranges: [{
                from: -100,
                to: -46,
                color: '#e64141'
              }, {
                from: -45,
                to: 0,
                color: '#089bab'
              }, {
                from: 0,
                to: 20,
                color: '#FC9F5B'
              }]
            },
            columnWidth: '80%',
          }
        },
        dataLabels: {
          enabled: false,
        },
        yaxis: {
          title: {
            text: 'Growth',
          },
          labels: {
            formatter: function (y) {
              return y.toFixed(0) + "%";
            }
          }
        },
        xaxis: {
          type: 'datetime',
          categories: [
            '2011-01-01', '2011-02-01', '2011-03-01', '2011-04-01', '2011-05-01', '2011-06-01',
            '2011-07-01', '2011-08-01', '2011-09-01', '2011-10-01', '2011-11-01', '2011-12-01',
            '2012-01-01', '2012-02-01', '2012-03-01', '2012-04-01', '2012-05-01', '2012-06-01',
            '2012-07-01', '2012-08-01', '2012-09-01', '2012-10-01', '2012-11-01', '2012-12-01',
            '2013-01-01', '2013-02-01', '2013-03-01', '2013-04-01', '2013-05-01', '2013-06-01',
            '2013-07-01', '2013-08-01', '2013-09-01'
          ],
          labels: {
            rotate: -90
          }
        }
        };

        var chart = new ApexCharts(document.querySelector("#home-servey-chart"), options);
        chart.render();
  });
   jQuery("#patient-chart-1").length && am4core.ready(function() {
    var options = {
          series: [{
          name: 'Servings',
          data: [44, 55, 41, 67, 22, 43, 21, 33, 45, 31]
        }],
          annotations: {
          points: [{
            x: 'Bananas',
            seriesIndex: 0,
            label: {
              borderColor: '#775DD0',
              offsetY: 0,
              style: {
                color: '#fff',
                background: '#775DD0',
              },
              text: 'Bananas are good',
            }
          }]
        },
        chart: {
          height: 350,
          type: 'bar',
        },
        colors:['#089bab'],
        plotOptions: {
          bar: {
            columnWidth: '50%',
            endingShape: 'rounded'  
          }
        },
        dataLabels: {
          enabled: false
        },
        stroke: {
          width: 2
        },
        
        grid: {
          row: {
            colors: ['#fff', '#f2f2f2']
          }
        },
        xaxis: {
          labels: {
            rotate: -45
          },
          categories: ['A', 'B', 'C', 'D', 'E', 'F',
            'G', 'H', 'I', 'J'
          ],
          tickPlacement: 'on'
        },
        yaxis: {
          title: {
            text: 'Servings',
          },
        },
        fill: {
          type: 'gradient',
          gradient: {
            shade: 'light',
            type: "horizontal",
            shadeIntensity: 0.25,
            gradientToColors: undefined,
            inverseColors: true,
            opacityFrom: 0.85,
            opacityTo: 0.85,
            stops: [50, 0, 100]
          },
        }
        };

        var chart = new ApexCharts(document.querySelector("#patient-chart-1"), options);
        chart.render();
   });

   jQuery("#doc-chart-01").length && am4core.ready(function() {
    am4core.ready(function() {

    // Themes begin
    am4core.useTheme(am4themes_animated);
    // Themes end


    var chart = am4core.create("doc-chart-01", am4charts.RadarChart);

    chart.data = [{
     "country": "USA",
     "visits": 2025
    }, {
     "country": "China",
     "visits": 1882
    }, {
     "country": "Japan",
     "visits": 1809
    }, {
     "country": "Germany",
     "visits": 1322
    }, {
     "country": "UK",
     "visits": 1122
    }, {
     "country": "France",
     "visits": 1114
    }, {
     "country": "India",
     "visits": 984
    }, {
     "country": "Spain",
     "visits": 711
    }, {
     "country": "Netherlands",
     "visits": 665
    }, {
     "country": "Russia",
     "visits": 580
    }, {
     "country": "South Korea",
     "visits": 443
    }, {
     "country": "Canada",
     "visits": 441
    }];

    chart.innerRadius = am4core.percent(40)

    var categoryAxis = chart.xAxes.push(new am4charts.CategoryAxis());
    categoryAxis.renderer.grid.template.location = 0;
    categoryAxis.dataFields.category = "country";
    categoryAxis.renderer.minGridDistance = 60;
    categoryAxis.renderer.inversed = true;
    categoryAxis.renderer.labels.template.location = 0.5;
    categoryAxis.renderer.grid.template.strokeOpacity = 0.08;

    var valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
    valueAxis.min = 0;
    valueAxis.extraMax = 0.1;
    valueAxis.renderer.grid.template.strokeOpacity = 0.08;

    chart.seriesContainer.zIndex = -10;


    var series = chart.series.push(new am4charts.RadarColumnSeries());
    series.dataFields.categoryX = "country";
    series.dataFields.valueY = "visits";
    series.tooltipText = "{valueY.value}"
    series.columns.template.strokeOpacity = 0;
    series.columns.template.radarColumn.cornerRadius = 5;
    series.columns.template.radarColumn.innerCornerRadius = 0;
    chart.colors.list = [am4core.color("#279fac"),
            am4core.color("#ffb57e"),
            am4core.color("#279fac"),
            am4core.color("#ffb57e"),
            am4core.color("#279fac"),
            am4core.color("#ffb57e"),
            am4core.color("#279fac"),
            am4core.color("#ffb57e"),
            am4core.color("#279fac"),
            am4core.color("#ffb57e"),
            am4core.color("#279fac"),
            am4core.color("#ffb57e")
        ];

    chart.zoomOutButton.disabled = true;

    // as by default columns of the same series are of the same color, we add adapter which takes colors from chart.colors color set
    series.columns.template.adapter.add("fill", (fill, target) => {
     return chart.colors.getIndex(target.dataItem.index);
    });

    setInterval(()=>{
     am4core.array.each(chart.data, (item)=>{
       item.visits *= Math.random() * 0.5 + 0.5;
       item.visits += 10;
     })
     chart.invalidateRawData();
    }, 2000)

    categoryAxis.sortBySeries = series;

    chart.cursor = new am4charts.RadarCursor();
    chart.cursor.behavior = "none";
    chart.cursor.lineX.disabled = true;
    chart.cursor.lineY.disabled = true;

    });
   });

/*---------------------------------------------------------------------
   circle progress bar
-----------------------------------------------------------------------*/
$(function() {

    $(".progress-round").each(function() {

        var value = $(this).attr('data-value');
        var left = $(this).find('.progress-left .progress-bar');
        var right = $(this).find('.progress-right .progress-bar');

        if (value > 0) {
            if (value <= 50) {
                right.css('transform', 'rotate(' + percentageToDegrees(value) + 'deg)')
            } else {
                right.css('transform', 'rotate(180deg)')
                left.css('transform', 'rotate(' + percentageToDegrees(value - 50) + 'deg)')
            }
        }

    })

    function percentageToDegrees(percentage) {

        return percentage / 100 * 360

    }

});



/*---------------------------------------------------------------------
Form Validation
-----------------------------------------------------------------------*/


window.addEventListener('load', function() {
    var forms = document.getElementsByClassName('needs-validation');
    var validation = Array.prototype.filter.call(forms, function(form) {
        form.addEventListener('submit', function(event) {
            if (form.checkValidity() === false) {
                event.preventDefault();
                event.stopPropagation();
            }
            form.classList.add('was-validated');
        }, false);
    });
}, false);



/*---------------------------------------------------------------------
    Form Wizard - 1
-----------------------------------------------------------------------*/

var current_fs, next_fs, previous_fs;
var opacity;
var current = 1;
var steps = jQuery("fieldset").length;

setProgressBar(current);

$(".next").click(function() {

    current_fs = $(this).parent();
    next_fs = $(this).parent().next();


    jQuery("#top-tab-list li").eq(jQuery("fieldset").index(next_fs)).addClass("active");
    jQuery("#top-tab-list li").eq(jQuery("fieldset").index(current_fs)).addClass("done");


    next_fs.show();
    current_fs.animate({
        opacity: 0
    }, {
        step: function(now) {
            opacity = 1 - now;

            current_fs.css({
                'display': 'none',
                'position': 'relative',

            });

            next_fs.css({
                'opacity': opacity
            });
        },
        duration: 500
    });
    setProgressBar(++current);
});

jQuery(".previous").click(function() {

    current_fs = $(this).parent();
    previous_fs = $(this).parent().prev();


    jQuery("#top-tab-list li").eq(jQuery("fieldset").index(current_fs)).removeClass("active");
    jQuery("#top-tab-list li").eq(jQuery("fieldset").index(previous_fs)).removeClass("done");


    previous_fs.show();

    current_fs.animate({
        opacity: 0
    }, {
        step: function(now) {
            opacity = 1 - now;

            current_fs.css({
                'display': 'none',
                'position': 'relative'
            });
            previous_fs.css({
                'opacity': opacity
            });
        },
        duration: 500
    });
    setProgressBar(--current);
});

function setProgressBar(curStep) {
    var percent = parseFloat(100 / steps) * curStep;
    percent = percent.toFixed();
    jQuery(".progress-bar")
        .css("width", percent + "%")
}

jQuery(".submit").click(function() {
    return false;
})


/*---------------------------------------------------------------------
   validate form wizard
-----------------------------------------------------------------------*/

var navListItems = jQuery('div.setup-panel div a'),
    allWells = jQuery('.setup-content'),
    allNextBtn = jQuery('.nextBtn');

allWells.hide();

navListItems.click(function(e) {
    e.preventDefault();
    var $target = jQuery(jQuery(this).attr('href')),
        $item = jQuery(this);

    if (!$item.hasClass('disabled')) {
        navListItems.addClass('active');
        $item.parent().addClass('active');
        allWells.hide();
        $target.show();
        $target.find('input:eq(0)').focus();
    }
});

allNextBtn.click(function() {
    var curStep = jQuery(this).closest(".setup-content"),
        curStepBtn = curStep.attr("id"),
        nextStepWizard = jQuery('div.setup-panel div a[href="#' + curStepBtn + '"]').parent().next().children("a"),
        curInputs = curStep.find("input[type='text'],input[type='email'],input[type='password'],input[type='url'],textarea"),
        isValid = true;

    jQuery(".form-group").removeClass("has-error");
    for (var i = 0; i < curInputs.length; i++) {
        if (!curInputs[i].validity.valid) {
            isValid = false;
            jQuery(curInputs[i]).closest(".form-group").addClass("has-error");
        }
    }

    if (isValid)
        nextStepWizard.removeClass('disabled').trigger('click');
});

jQuery('div.setup-panel div a.active').trigger('click');

/*---------------------------------------------------------------------
   Vertical form wizard
-----------------------------------------------------------------------*/


var current_fs, next_fs, previous_fs; //fieldsets
var opacity;
var current = 1;
var steps = jQuery("fieldset").length;

setProgressBar(current);

$(".next").click(function() {

    current_fs = $(this).parent();
    next_fs = $(this).parent().next();


    jQuery("#top-tabbar-vertical li").eq(jQuery("fieldset").index(next_fs)).addClass("active");


    next_fs.show();
    current_fs.animate({
        opacity: 0
    }, {
        step: function(now) {
            opacity = 1 - now;

            current_fs.css({
                'display': 'none',
                'position': 'relative'
            });
            next_fs.css({
                'opacity': opacity
            });
        },
        duration: 500
    });
    setProgressBar(++current);
});

jQuery(".previous").click(function() {

    current_fs = $(this).parent();
    previous_fs = $(this).parent().prev();

    jQuery("#top-tabbar-vertical li").eq(jQuery("fieldset").index(current_fs)).removeClass("active");

    previous_fs.show();

    current_fs.animate({
        opacity: 0
    }, {
        step: function(now) {
            opacity = 1 - now;

            current_fs.css({
                'display': 'none',
                'position': 'relative'
            });
            previous_fs.css({
                'opacity': opacity
            });
        },
        duration: 500
    });
    setProgressBar(--current);
});

function setProgressBar(curStep) {
    var percent = parseFloat(100 / steps) * curStep;
    percent = percent.toFixed();
    jQuery(".progress-bar")
        .css("width", percent + "%")
}

jQuery(".submit").click(function() {
    return false;
})

