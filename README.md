# YOYOW.NFT
yoyow contract of NFT

��Լ�Ĺ���Ա���Թ̶�ָ���ʲ������ޣ�Ĭ��Ϊ28182�������Ҫ���޸Ĵ����е�SYSTEM_MANAGER_UIDΪ����uid

�����в��Բ���˵��
0,�����Լ
gxx -g nft.abi nft.cpp
gxx -o nft.wast nft.cpp

1,���ú�Լ���Ƚ�����0�����nft.abi��nft.wasm������yoyow_client����Ŀ¼��nft��Ŀ¼�ڣ�
deploy_contract 28182   0 0 ./nft false true

2,��ʼ��
call_contract  28182  28182  null init  "{\"version\":\"0\"}" false true

3,����NFT
call_contract  28182  28182  null create  "{\"issuer\":\"28182\", \"category\":\"tokentypes\",\"tkname\":\"testnft\",\"burnable\":\"1\",\"transferable\":\"1\",\"uri\":\"string\",\"max_issue_days\":\"30\",\"max_supply\":\"10000\"}" false true

4,�ַ�NFT
call_contract  28182  28182  null issue  "{\"to\":\"25997\",\"category\":\"tokentypes\", \"tkname\":\"testnft\",\"quantity\":\"1\",\"relative_uri\":\"http://www.baidu.com\",\"memo\":\"testmemo1\"}" false true
call_contract  28182  28182  null issue  "{\"to\":\"25997\",\"category\":\"tokentypes\", \"tkname\":\"testnft\",\"quantity\":\"1\",\"relative_uri\":\"http://www.baidu2.com\",\"memo\":\"testmemo2\"}" false true

5,����NFT
call_contract  25997  28182  null burn  "{\"owner\":\"25997\",\"item_ids\":[2]}" false true


6,ת��NFT
call_contract  25997  28182  null transfer  "{\"from\":\"25997\",\"to\":\"28182\",\"item_ids\":[1],\"memo\":\"testmemo3\"}" false true

7,��������,����������
call_contract  28182  28182  null fixmaxsupply  "{\"category\":\"tokentypes\",\"tkname\":\"testnft\"}" false true

*****��ѯ��ز���*****
��ѯABI:
get_account init10

��ѯ��Լ����Щ��
get_contract_tables init10

��ѯ�Ѿ��ַ���NFT
1,����id��ѯ
get_table_rows_ex  init10  item {"lower_bound":0,"upper_bound":-1,"limit":10,"index_position":"1","reverse":false}

2,���ݳ����߲�ѯ(lower_bound��Ϊ�����ߵ�uid)
get_table_rows_ex  init10  item {"lower_bound":28182,"upper_bound":-1,"limit":10,"index_position":"2","reverse":false}

��ѯ�˺�28182��token���
get_table_objects  28182  28182  false accounts  0   -1  100

��ѯcategoryΪ"tokentypes"��token
get_table_objects  28182  tokentypes  true itemstats  0   -1  100

���º�Լ
update_contract  init10 ./nft false true
