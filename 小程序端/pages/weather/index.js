var amapFile = require('../../utils/amap-wx.js');//如：..­/..­/libs/amap-wx.js

Page({
  onLoad: function() {
    var that = this;
    var myAmapFun = new amapFile.AMapWX({key:'81fea59d06f68c90005fa6385c819a04'});
    myAmapFun.getWeather({
      success: function(data){
        //成功回调
        console.log(data)

      },
      fail: function(info){
        //失败回调
        console.log(info)
      }
    })
  }
})