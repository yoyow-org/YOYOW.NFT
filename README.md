# YOYOW.NFT
yoyow contract of NFT

合约的管理员可以固定指定资产的上限，默认为28182，如果需要请修改代码中的SYSTEM_MANAGER_UID为其他uid

命令行测试步骤说明
0,编译合约
gxx -g nft.abi nft.cpp
gxx -o nft.wast nft.cpp

1,布置合约（先将步骤0编译的nft.abi和nft.wasm拷贝至yoyow_client所在目录的nft子目录内）
deploy_contract 28182   0 0 ./nft false true

2,初始化
call_contract  28182  28182  null init  "{\"version\":\"0\"}" false true

3,创建NFT
call_contract  28182  28182  null create  "{\"issuer\":\"28182\", \"category\":\"tokentypes\",\"tkname\":\"testnft\",\"burnable\":\"1\",\"transferable\":\"1\",\"uri\":\"string\",\"max_issue_days\":\"30\",\"max_supply\":\"10000\"}" false true

4,分发NFT
call_contract  28182  28182  null issue  "{\"to\":\"25997\",\"category\":\"tokentypes\", \"tkname\":\"testnft\",\"quantity\":\"1\",\"relative_uri\":\"http://www.baidu.com\",\"memo\":\"testmemo1\"}" false true
call_contract  28182  28182  null issue  "{\"to\":\"25997\",\"category\":\"tokentypes\", \"tkname\":\"testnft\",\"quantity\":\"1\",\"relative_uri\":\"http://www.baidu2.com\",\"memo\":\"testmemo2\"}" false true

5,销毁NFT
call_contract  25997  28182  null burn  "{\"owner\":\"25997\",\"item_ids\":[2]}" false true


6,转账NFT
call_contract  25997  28182  null transfer  "{\"from\":\"25997\",\"to\":\"28182\",\"item_ids\":[1],\"memo\":\"testmemo3\"}" false true

7,锁定上限,不可再增发
call_contract  28182  28182  null fixmaxsupply  "{\"category\":\"tokentypes\",\"tkname\":\"testnft\"}" false true

*****查询相关操作*****
查询ABI:
get_account init10

查询合约有哪些表
get_contract_tables init10

查询已经分发的NFT
1,根据id查询
get_table_rows_ex  init10  item {"lower_bound":0,"upper_bound":-1,"limit":10,"index_position":"1","reverse":false}

2,根据持有者查询(lower_bound设为持有者的uid)
get_table_rows_ex  init10  item {"lower_bound":28182,"upper_bound":-1,"limit":10,"index_position":"2","reverse":false}

查询账号28182的token余额
get_table_objects  28182  28182  false accounts  0   -1  100

查询category为"tokentypes"的token
get_table_objects  28182  tokentypes  true itemstats  0   -1  100

更新合约
update_contract  init10 ./nft false true
