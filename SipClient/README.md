# SipClient

依赖[ZLMediaKit](https://github.com/ZLMediaKit/ZLMediaKit)实现的一个简单的 GB28181 设备端。

## 实现功能

- 设备注册
- 实时预览
- 心跳
- 目录查询
- 文件检索和回放
- 其他消息响应，DeviceInfo、ConfigDownload等。


## 依赖库：

- [osip](https://www.gnu.org/software/osip/osip.html)
- [eXosip2](http://savannah.nongnu.org/projects/exosip)
- [openssl](https://github.com/openssl/openssl)
- [glog](https://github.com/google/glog)
- [curl](https://github.com/curl/curl)
- [cpr](https://github.com/libcpr/cpr)
- [fmt](https://github.com/fmtlib/fmt)
- [pugixml](https://github.com/zeux/pugixml)
- [magic_enum](https://github.com/Neargye/magic_enum)

## 配置文件

```xml
<?xml version="1.0" encoding="utf-8" ?>
<Config>
	<SipServer>
        <!-- 服务器IP地址 -->
		<IP>172.20.5.38</IP>
        <!-- SIP协议端口 -->
		<Port>10000</Port>
        <!-- 密码 -->
		<Password>12345678</Password>
        <!-- 服务器ID -->
		<ID>34020000002000000001</ID>
	</SipServer>
	<MediaServer>
        <!-- ZLMediaKit地址 -->
		<IP>127.0.0.1</IP>
        <!-- ZLMediaKit的Http端口 -->
		<Port>10080</Port>
		<Secret>035c73f7-bb6b-4889-a715-d9eb2d1925cc</Secret>
	</MediaServer>
    <HttpServer>
		<!-- Http服务端口，用于回放时，接收ZLM文件流注销事件 -->
		<Port>28080</Port>
	</HttpServer>
	<DeviceList>
		<Device>
            <!-- 本地IP -->
			<IP>192.168.116.125</IP>
            <!-- 本地SIP使用端口，这个不重要 -->
			<Port>15060</Port>
            <!-- 设备ID -->
			<ID>34020000002000000001</ID>
            <!-- SIP注册使用协议，TCP or UDP -->
			<Protocol>TCP</Protocol>
            <!-- 设备名称 -->
			<Name>IP Camera</Name>
            <!-- 厂家 -->
			<Manufacturer>Test</Manufacturer>
            <!-- 心跳间隔，单位：s -->
			<HeartbeatInterval>60</HeartbeatInterval>
			<Catalog>
                <!-- 当前设备的通道信息，可以有多个 -->
				<Channel>
                    <!-- 通道名称 -->
					<Name>Test_1</Name>
                    <!-- 通道编码 -->
					<ID>34020000002000000011</ID>
                    <!-- 此通道对应ZLM中的流地址 -->
					<URI>h265/ch1/sub/av_stream</URI>
				</Channel>
			</Catalog>
		</Device>
	</DeviceList>
</Config>
```
