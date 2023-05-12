import mqtt from'../../utils/mqtt.js';
const aliyunOpt = require('../../utils/aliyun/aliyun_connect.js');

let that = null;
Page({
    data:{

      //设置温度值和湿度值 
      temperature:"",
      humidity:"",
      GasConcentration:"",
      RunningState:"",
      openstate:"",
      client:null,//记录重连的次数
      reconnectCounts:0,//MQTT连接的配置
      options:{
        protocolVersion: 4, //MQTT连接协议版本
        clean: false,
        reconnectPeriod: 1000, //1000毫秒，两次重新连接之间的间隔
        connectTimeout: 30 * 1000, //1000毫秒，两次重新连接之间的间隔
        resubscribe: true, //如果连接断开并重新连接，则会再次自动订阅已订阅的主题（默认true）
        clientId: '',
        password: '',
        username: '',
      },

      aliyunInfo: {
        productKey: 'idxoCkRSquB', //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
        deviceName: 'WeiXinDemo', //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
        deviceSecret: 'e379bb0f7a3e8d9dabdb3baf23df415a', //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
        regionId: 'cn-shanghai', //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
        pubTopic: '/idxoCkRSquB/WeiXinDemo/user/WeiXinDuan', //发布消息的主题
        subTopic: '/sys/idxoCkRSquB/WeiXinDemo/thing/service/property/set', //订阅消息的主题
      },
    },

  onLoad:function(){
    that = this;
    let clientOpt = aliyunOpt.getAliyunIotMqttClient({
      productKey: that.data.aliyunInfo.productKey,
      deviceName: that.data.aliyunInfo.deviceName,
      deviceSecret: that.data.aliyunInfo.deviceSecret,
      regionId: that.data.aliyunInfo.regionId,
      port: that.data.aliyunInfo.port,
    });

    console.log("get data:" + JSON.stringify(clientOpt));
    let host = 'wxs://' + clientOpt.host;
    
    this.setData({
      'options.clientId': clientOpt.clientId,
      'options.password': clientOpt.password,
      'options.username': clientOpt.username,
    })
    console.log("this.data.options host:" + host);
    console.log("this.data.options data:" + JSON.stringify(this.data.options));

    //访问服务器
    this.data.client = mqtt.connect(host, this.data.options);

    this.data.client.on('connect', function (connack) {
      wx.showToast({
        title: '连接成功'
      })
      console.log("连接成功");
    })

    //接收消息监听
    this.data.client.on("message", function (topic, payload) {
      console.log(" 收到 topic:" + topic + " , payload :" + payload);
      that.setData({
        //转换成JSON格式的数据进行读取
         //temperature:JSON.parse(payload).items.temperature.value,
        // humidity:JSON.parse(payload).items.Humidity.value,
        // RunningState:JSON.parse(payload).items.RunningState.value,
        // GasConcentration:JSON.parse(payload).items.GasConcentration.value,
        /* 收到 topic:/idxoCkRSquB/WeiXinDemo/user/get , payload :{"deviceType":"CustomCategory","iotId":"MeqdTV27NUsrl775fT2Oidxo00","requestId":"123","checkFailedData":{},"productKey":"idxoCkRSquB","gmtCreate":1681973474501,"deviceName":"device","items":{"RunningState":{"time":1681973474474,"value":1},"Humidity":{"time":1681973474474,"value":58.9},"temperature":{"time":1681973474474,"value":28.7}}}*/
         temperature:JSON.parse(payload).temperature,
         humidity:JSON.parse(payload).humidity,
         RunningState:JSON.parse(payload).RunningState,
          GasConcentration:JSON.parse(payload).gas,
      })
      if(that.data.RunningState==1){
        that.setData({
          openstate:"有人"
        })
      }else{
        that.setData({
          openstate:"无人"
        })
      }
     /* wx.showModal({
        content: " 收到topic:[" + topic + "], payload :[" + payload + "]",
        showCancel: false,
      }); */
    })

    //服务器连接异常的回调
    that.data.client.on("error", function (error) {
      console.log(" 服务器 error 的回调" + error)

    })
    //服务器重连连接异常的回调
    that.data.client.on("reconnect", function () {
      console.log(" 服务器 reconnect的回调")

    })
    //服务器连接异常的回调
    that.data.client.on("offline", function (errr) {
      console.log(" 服务器offline的回调")
    })
  },
  onWaterChange: function(event) {
    var that = this
    console.log(event.detail)
    let sw = event.detail.value
    // 要发布的主题

  },
  onClickDoorOpen() {
    that.sendCommond(1);
  },
  onClickFlashOpen() {
    that.sendCommond(2);
  },
  sendCommond(cmd) {
    let sendData = {
      RunningState: cmd,
    
    };

//此函数是订阅的函数，因为放在访问服务器的函数后面没法成功订阅topic，因此把他放在这个确保订阅topic的时候已成功连接服务器
//订阅消息函数，订阅一次即可 如果云端没有订阅的话，需要取消注释，等待成功连接服务器之后，在随便点击（开灯）或（关灯）就可以订阅函数
 /* this.data.client.subscribe(this.data.aliyunInfo.subTopic,function(err){
      if(!err){
        console.log("订阅成功");
      };
      wx.showModal({
        content: "订阅成功",
        showCancel: false,
      })
    })  
    */

    //发布消息
    if (this.data.client && this.data.client.connected) {
      this.data.client.publish(this.data.aliyunInfo.pubTopic, JSON.stringify(sendData));
      console.log("************************")
      console.log(this.data.aliyunInfo.pubTopic)
      console.log(JSON.stringify(sendData))
    } else {
      wx.showToast({
        title: '请先连接服务器',
        icon: 'none',
        duration: 2000
      })
    }
  }
})