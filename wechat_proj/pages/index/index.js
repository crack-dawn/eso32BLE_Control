// index.js
// 获取应用实例
 

/**
* 检测是否有对应的权限，通过回调函数返回结果
* @param {String} perName 权限名称
* @param {function} perResultCbFun 结果回调函数，参数为true表示成功
*/
function permission_check(perName, perResultCbFun) {
    // var arr = (typeof perArr != "object") ? [perArr] : perArr; // 确保参数总是数组
    wx.getSetting({
      success(res) {
        if (!res.authSetting[perName]) {
            if (typeof perResultCbFun == "function") {
              console.log("授权状态获取失败", perName);
              perResultCbFun(false);
            }
        } else {
            if (typeof perResultCbFun == "function") {
              console.log("授权状态获取成功", perName);
              perResultCbFun(true);
            }
        }
      },
      fail(res) {
        console.log("授权状态获取失败", perName);
        if (typeof perResultCbFun == "function") {
          perResultCbFun(false);
        }
      }
    })
  }

  /**
* 请求对应的权限
* @param {String} perName 权限名称
* @param {String} perZhName 权限对应的中文名称，用来做提示用
* @param {function} perRequestCbFun 请求结果回调（参数为true表示成功）
*/
function permission_request(perName, perZhName, perRequestCbFun) {
    permission_check(perName,
      (perCheckResualt) => {
        if (perCheckResualt) {  // 权限已经请求成功
          if (typeof perRequestCbFun == "function") {
            perRequestCbFun(true);
          }
        } else {
          // 如果没有该权限，就去申请该权限
          wx.authorize({
            scope: perName,
            success () {
              // 用户已经同意小程序使用ble，后续调用 wx.startRecord 接口不会弹窗询问
              if (typeof perRequestCbFun == "function") {
                perRequestCbFun(true);
              }
            },
            fail() {
              // 用户拒绝授予权限
              console.log("用户拒绝授权", perName);
              // 弹出提示框，提示用户需要申请权限
              wx.showModal({
                title: '申请权限',
                content: '需要使用'+perZhName+'权限，请前往设置打开权限',
                success(res) {
                  if (res.confirm) {
                    console.log('用户点击确定');
                    // 打开权限设置页面，即使打开了权限界面，也不知道用户是否打开了权限，所以这里返回失败
                    wx.openSetting({
                      success(res) {
                        if (typeof perRequestCbFun == "function") {
                          perRequestCbFun(false);
                        }
                      },
                      fail(err) {
                        if (typeof perRequestCbFun == "function") {
                          perRequestCbFun(false);
                        }
                      }
                    })
                  } else if (res.cancel) {
                    if (typeof perRequestCbFun == "function") {
                      perRequestCbFun(false);
                    }
                  }
                }
              })
            }
          })
        }
      }
    )
  }
 
 
const blePermissionName = 'scope.bluetooth'; // 蓝牙对应的权限名称
const blePermissionZhName = '蓝牙'; // 蓝牙权限对应的中文名称  
 
const app = getApp();

Page({
  data: {
    'deviceId':'',
    'serviceId':'',
    'characteristicId':''
  },
  onLoad() {
    permission_check(blePermissionName, blePermissionZhName);
    permission_request(blePermissionName, blePermissionZhName);
    this.bleInit();
  },
  bleInit() {//初始化蓝牙
    console.log('searchBle')
    // 监听扫描到新设备事件
    wx.onBluetoothDeviceFound((res) => {
      res.devices.forEach((device) => {
        // 这里可以做一些过滤
        console.log('Device Found', device)
        if(device.deviceId == "EC:62:60:9D:50:A2"){    // 
          // 找到设备开始连接
          this.bleConnection(device.deviceId);
          wx.stopBluetoothDevicesDiscovery()
        }
      })
      // 找到要搜索的设备后，及时停止扫描
      // 
    })

    // 初始化蓝牙模块
    wx.openBluetoothAdapter({
      mode: 'central',
      success: (res) => {
        // 开始搜索附近的蓝牙外围设备
        wx.startBluetoothDevicesDiscovery({
          allowDuplicatesKey: false,
        })
      },
      fail: (res) => {
        if (res.errCode !== 10001) return
        wx.onBluetoothAdapterStateChange((res) => {
          if (!res.available) return
          // 开始搜寻附近的蓝牙外围设备
          wx.startBluetoothDevicesDiscovery({
            allowDuplicatesKey: false,
          })
        })
      }
    })
    var that = this
    wx.onBLECharacteristicValueChange((result) => {
      console.log('onBLECharacteristicValueChange',result.value)
      let hex = that.ab2hex(result.value)
      console.log('hextoString',that.hextoString(hex))
      console.log('hex',hex)
    })
  },
  bleConnection(deviceId){
    wx.createBLEConnection({
      deviceId, // 搜索到设备的 deviceId
      success: () => {
        console.log('连接成功，获取服务'+deviceId)
        this.bleGetDeviceServices(deviceId) //设备服务函数
      }
    })
  },
  bleGetDeviceServices(deviceId){
    wx.getBLEDeviceServices({
      deviceId, // 搜索到设备的 deviceId
      success: (res) => {
        console.log(res.services)
        for (let i = 0; i < res.services.length; i++) {
          if (res.services[i].isPrimary) {
            // 可根据具体业务需要，选择一个主服务进行通信
            this.bleGetDeviceCharacteristics(deviceId,res.services[i].uuid)
          }
        }
      }
    })
  },
  bleGetDeviceCharacteristics(deviceId,serviceId){
    wx.getBLEDeviceCharacteristics({
      deviceId, // 搜索到设备的 deviceId
      serviceId, // 上一步中找到的某个服务
      success: (res) => {
        for (let i = 0; i < res.characteristics.length; i++) {
          let item = res.characteristics[i]
          console.log(item)
          if (item.properties.write) { // 该特征值可写
            // 本示例是向蓝牙设备发送一个 0x00 的 16 进制数据
            // 实际使用时，应根据具体设备协议发送数据
            // let buffer = new ArrayBuffer(1)
            // let dataView = new DataView(buffer)
            // dataView.setUint8(0, 0)
            // let senddata = 'FF';
            // let buffer = this.hexString2ArrayBuffer(senddata);
            var buffer = this.stringToBytes("getid")
            this.setData({
              'deviceId':deviceId,
              'serviceId':serviceId,
              'characteristicId':item.uuid
            })
            wx.writeBLECharacteristicValue({
              deviceId,
              serviceId,
              characteristicId: item.uuid,
              value: buffer,
            })
          }
          if (item.properties.read) { // 改特征值可读
            wx.readBLECharacteristicValue({
              deviceId,
              serviceId,
              characteristicId: item.uuid,
            })
          }
          if (item.properties.notify || item.properties.indicate) {
            // 必须先启用 wx.notifyBLECharacteristicValueChange 才能监听到设备 onBLECharacteristicValueChange 事件
            wx.notifyBLECharacteristicValueChange({
              deviceId,
              serviceId,
              characteristicId: item.uuid,
              state: true,
            })
          }
        }
      }
    })
  },
  stringToBytes(str) {
    var array = new Uint8Array(str.length);
    for (var i = 0, l = str.length; i < l; i++) {
      array[i] = str.charCodeAt(i);
    }
    console.log(array);
    return array.buffer;
  },
  hextoString: function (hex) {
    var arr = hex.split("")
    var out = ""
    for (var i = 0; i < arr.length / 2; i++) {
      var tmp = "0x" + arr[i * 2] + arr[i * 2 + 1]
      var charValue = String.fromCharCode(tmp);
      out += charValue
    }
    return out
  },
  ab2hex(buffer) {
    var hexArr = Array.prototype.map.call(
      new Uint8Array(buffer),
      function (bit) {
        return ('00' + bit.toString(16)).slice(-2)
      }
    )
    return hexArr.join('');
  },
  light1on(){
 
    var buffer = this.stringToBytes("on")
    wx.writeBLECharacteristicValue({
      deviceId:this.data.deviceId,
      serviceId:this.data.serviceId,
      characteristicId:this.data.characteristicId,
      value: buffer,
    })
  },
  light2on(){
    var buffer = this.stringToBytes("light2on")
    wx.writeBLECharacteristicValue({
      deviceId:this.data.deviceId,
      serviceId:this.data.serviceId,
      characteristicId:this.data.characteristicId,
      value: buffer,
    })
  },
  light1off(){
    var buffer = this.stringToBytes("off")
    wx.writeBLECharacteristicValue({
      deviceId:this.data.deviceId,
      serviceId:this.data.serviceId,
      characteristicId:this.data.characteristicId,
      value: buffer,
    })
  },
  light2off(){
    var buffer = this.stringToBytes("light2off")
    wx.writeBLECharacteristicValue({
      deviceId:this.data.deviceId,
      serviceId:this.data.serviceId,
      characteristicId:this.data.characteristicId,
      value: buffer,
    })
  },
  
})

