Page
({
  data: 
  {
    rx:"",
    tx:"",
    T:"",
    H:"",
    info:"",                //显示框
    name:"BLE_test",

    connectedDeviceId:"",   //连接上的设备的ID
    devices:"",             //所有搜索到的蓝牙设备
    services: "" ,          //连接上设备的所有服务
    notifyCharacteristicsId: "", //响应UUID，根据蓝牙模块获得
    writeCharacteristicsId:  "", //写UUID， 根据蓝牙模块获得
  },
 
  
  /****************1.蓝牙初始化***************/
  BLEInit( )
  {
      wx.openBluetoothAdapter
      ({
          success: res=>
          {
            console.log('初始化蓝牙适配器成功')
            this.setData({info: '初始化蓝牙适配器成功'})
            setTimeout(()=> {this.BLEState( );}, 50)
          },
          fail: res=>
          {
            console.log('请打开蓝牙和定位功能')
            this.setData({info: '请打开蓝牙和定位功能'})
          }
      })
  },
  /****************2.获取蓝牙状态***************/
  BLEState()
  {
    wx.getBluetoothAdapterState
    ({
        success: res=>
        {
          //打印相关信息
          console.log(JSON.stringify(res.errMsg) + "\n蓝牙是否可用：" + res.available);
          setTimeout(()=>{this.BLESearch()}, 1500)
          this.setData({info: JSON.stringify(res.errMsg) +"\n蓝牙是否可用：" + res.available})
        },

        fail: res=>
        {
          console.log(JSON.stringify(res.errMsg) + "\n蓝牙是否可用：" + res.available);
          this.setData({info: JSON.stringify(res.errMsg) + "\n蓝牙是否可用：" + res.available})
        }
    })
 
  },
  /****************3.搜索设备**************/
  BLESearch()
  {
    console.log("搜索设备\n");
    wx.startBluetoothDevicesDiscovery
    ({
 //如果填写了此UUID，那么只会搜索出含有这个UUID的设备，建议一开始先不填写
        success: res=>
        {
        setTimeout(()=>{ this.BLEGetDevices()}, 3500)
          this.setData({info: "搜索设备中...." ,})
          console.log('搜索设备返回' + JSON.stringify(res))
        }
    })
  },
  /****************4.获取蓝牙设备***************/
  BLEGetDevices()
  {
    console.log("获取蓝牙设备\n");
    wx.getBluetoothDevices
    ({
        success: res=>
        {
          this.setData({
            info: "获取设备信息中，可连接设备数量:" +res.devices.length,
            devices: res.devices})
            setTimeout(()=>{this.BLEConnect()}, 1000)
          console.log('搜设备数目：' + res.devices.length)
          console.log('设备信息：\n' + JSON.stringify(res.devices)+"\n")
        }
    })
  },
   /****************5.连接蓝牙设备***************/
  BLEConnect()
  {
    console.log("连接蓝牙设备\n");
    this.data.devices.forEach(device => {
        if (device.name == 'BLE_test' || device.deviceId == "EC:62:60:9D:50:A2") {
            wx.createBLEConnection
            ({
              deviceId: device.deviceId,
              success: res=>
              {
                console.log('已连接设备：' + res.errMsg);
                this.setData({connectedDeviceId:  device.deviceId,name:device.name,info:"蓝牙设备已连接"})
                setTimeout(()=>{  this.BLESearchStop()}, 200)
              },
              fail: res=>{console.log("连接失败"+res.errMsg);},
            })
        }
    });
   
  },
  /****************6.停止搜索蓝牙设备***************/
  BLESearchStop()
  {
    console.log("停止搜索蓝牙设备\n");
    wx.stopBluetoothDevicesDiscovery
    ({
      success: res=>
      {
        console.log("停止搜索" + JSON.stringify(res.errMsg));
        setTimeout(()=>{  this.BLEGetservice() }, 200)
        this.setData
        ({
          info: "停止搜索"  + JSON.stringify(res.errMsg),
          devices:"",
        })
      }
    })
  },
  /****************7.获取服务service***************/
  BLEGetservice()
  {
    console.log("获取服务service\n");
    wx.getBLEDeviceServices
    ({
      // 这里的 deviceId 需要在上面的 getBluetoothDevices 或 onBluetoothDeviceFound 接口中获取
      deviceId: this.data.connectedDeviceId,
      success: res=>
      {
        console.log('services UUID:\n', JSON.stringify(res.services));
        for (var i = 0; i < res.services.length; i++) 
        {
          console.log("第"+(i+1) + "个UUID:" + res.services[i].uuid+"\n")
        }
        this.setData
        ({
          services: res.services,
           info: JSON.stringify(res.services+"获取服务service成功"),
        })
        setTimeout(()=>{ this.BLEGetCharacteristics( ) }, 1500)
      }
    })
  },
  /****************8.获取所有的特征值***************/
  BLEGetCharacteristics( )
  {
    wx.getBLEDeviceCharacteristics
    ({
      // 这里的 deviceId 需要在上面的 getBluetoothDevices 中获取
      deviceId: this.data.connectedDeviceId,
      // 这里的 serviceId 需要在上面的 getBLEDeviceServices 接口中获取
      serviceId: this.data.services[0].uuid,
      success: res=>
      {
        console.log("%c getBLEDeviceCharacteristics", "color:red;");
        for (var i = 0; i < res.characteristics.length; i++) 
        {
          console.log('特征值：' + res.characteristics[i].uuid)
 
          if (res.characteristics[i].properties.notify  === true ) 
          {
            this.setData({notifyCharacteristicsId:res.characteristics[i].uuid})
            console.log("notifyID:"+res.characteristics[i].properties.uuid+'\n')
          }
          if (res.characteristics[i].properties.write   === true )
          {
            this.setData({writeCharacteristicsId:res.characteristics[i].uuid})
            console.log("wirteID:"+res.characteristics[i].properties.uuid+'\n')
          }
        }
        this.setData({info:"获取所有的特征值成功"})
        setTimeout(()=>{ this.BLECharacteristicValueChange( )}, 2000)
      },

      fail: res=>
      {
        console.log("获取所有特征值失败");
      },
    })
  },
  /****************9.启用特征值变化***************/
  BLECharacteristicValueChange( )
  {
    console.log("启用的notifyCharacteristicsId", this.data.notifyCharacteristicsId);
    wx.notifyBLECharacteristicValueChange
    ({
      state: true,
      deviceId: this.data.connectedDeviceId,
      serviceId: this.data.services[0].uuid,
      characteristicId: this.data.notifyCharacteristicsId,

      success: res=>
      {
        console.log('9.启用特征值变化成功：', res.errMsg)
        this.setData({info:"启用特征值变化成功"+res.errMsg})
        this.BLEDataRecieve( )
      },
      fail: res=>{console.log('启动notify失败:' + res.errMsg); },
      
    })
  },
  /****************10.接收蓝牙的返回数据***************/
  BLEDataRecieve( )
  {
    wx.onBLECharacteristicValueChange
    (res=>{
        console.log("消息长度:" + res.value.byteLength)
        console.log("接收蓝牙的返回数据:" + res.value)
        var the_rx =ab2hex(res.value)
        this.setData({
            rx: the_rx,
            T: the_rx[0],
            H: the_rx[1]
        })
        console.log()
      })
  },

/****************11.蓝牙发送数据***************/
  BLEDataSend(msgToSend)
  {
    wx.writeBLECharacteristicValue
    ({
      deviceId: this.data.connectedDeviceId,
      serviceId: this.data.services[0].uuid,
      characteristicId: this.data.writeCharacteristicsId,
      // 这里的value是ArrayBuffer类型
      value: stringToBytes(msgToSend),

      success: res=>{
          console.log('写入成功:', res.errMsg)
         this.setData({tx:msgToSend})
        },
      fail(res){console.log('写入失败:', res.errMsg)}
    })
  },
  /****************12.断开蓝牙的连接***************/
  BLEDisconnect()
  {
    wx.closeBLEConnection
    ({
        deviceId: this.data.connectedDeviceId,
        success: res=>
        {
          this.setData
          ({
            connectedDeviceId: "",
            T:"",
            H:"",
            rx:"",
            tx:"",
            info:"",                //显示框
            devices:"",
            services:""
          })
          console.log('断开蓝牙设备成功：' + res.errMsg)
        },
        fail:res=>
        {
          console.log('断开蓝牙设备失败：' + res.errMsg)
        }
    })
  },
  //获取输入框的数据
  getmsg(event){
    this.setData({sendmsg:event.detail.value})
  },
  send(){
    var msg =this.data.sendmsg
    this.BLEDataSend(msg)
    
  },
  button_function(e){
      var buffer =e.currentTarget.dataset.my
      this.BLEDataSend(buffer)  
  }
})


// 16进制数字转 字符串
function hextoStrin(hexx) {
    var arr = hexx.split("")
    var out = ""
    for (var i = 0; i < arr.length / 2; i++) {
      var tmp = "0x" + arr[i * 2] + arr[i * 2 + 1]
      var charValue = String.fromCharCode(tmp);
      out += charValue
    }
    return out
  }

// ArrayBuffer转16进度字符串示例
function ab2hex(buffer) {
  var hexArr = Array.prototype.map.call(
    new Uint8Array(buffer),
    function (bit) {
      return ('00' + bit.toString(16)).slice(-2)
    }
  )
  return hexArr.join(',');
}


function ab2str(buf) {
    return String.fromCharCode.apply(null, new Uint16Array(buf));
  }
  function str2ab(str) {
    var buf = new ArrayBuffer(str.length*2); // 2 bytes for each char
    var bufView = new Uint16Array(buf);
    for (var i=0, strLen=str.length; i < strLen; i++) {
      bufView[i] = str.charCodeAt(i);
    }
    return buf;
  }

function arrayBufferToString(arr){
    if(typeof arr === 'string') {  
        return arr;  
    }  
    var dataview=new DataView(arr.data);
    var ints=new Uint8Array(arr.data.byteLength);
    for(var i=0;i<ints.length;i++){
      ints[i]=dataview.getUint8(i);
    }
    arr=ints;
    var str = '',  
        _arr = arr;  
    for(var i = 0; i < _arr.length; i++) {  
        var one = _arr[i].toString(2),  
            v = one.match(/^1+?(?=0)/);  
        if(v && one.length == 8) {  
            var bytesLength = v[0].length;  
            var store = _arr[i].toString(2).slice(7 - bytesLength);  
            for(var st = 1; st < bytesLength; st++) {  
                store += _arr[st + i].toString(2).slice(2);  
            }  
            str += String.fromCharCode(parseInt(store, 2));  
            i += bytesLength - 1;  
        } else {  
            str += String.fromCharCode(_arr[i]);  
        }  
    }  
    return str; 
}
  
// 16进度字符串转ArrayBuffer 
function stringToBytes(str) {
    var array = new Uint8Array(str.length);
    for (var i = 0, l = str.length; i < l; i++) {
      array[i] = str.charCodeAt(i);
    }
    console.log(array);
    return array.buffer;
}