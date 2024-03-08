

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

  module.exports = {
    permission_check,
    permission_request
  }