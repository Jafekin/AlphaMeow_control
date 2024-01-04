# Balance Car v1.0

## 介绍

基于Hi3861的平衡车，各项功能完善，平衡性较差有待调参和结构优化，使用前需注意：
将`src\device\hisilicon\hispark_pegasus\sdk_liteos\platform\os\Huawei_LiteOS\targets\hi3861v100\include\target_config.h`
内的`LOSCFG_BASE_CORE_TICK_PER_SECOND`改为`1000UL`从而使tick由10ms提升为1ms以提升liteOS-M实时性

## 模块

- [x] bno055驱动
- [x] 编码器计数
- [x] 电机驱动
- [x] PID控制算法
- [x] UDP通信调参、数据回传
- [x] 主函数
- [x] 无线遥控模块
- [ ] PID调参  
