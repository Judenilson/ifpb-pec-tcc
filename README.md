# tcc
TCC


### personal_configs
- é uma pasta criada no diretorio raiz do projeto, ignorada no .gitignore que serve para guardar configurações com senhas que não devem ser compartilhadas, ex: wifi_router.cpp que guarda as informações de login da rede do roteador que é importado no código da main.cpp, dessa forma a build sempre vai estar configurada com o seu roteador sem expor seus dados no repositório público
- wifi_router.cpp
  ![](/documentation/wifi_router_config/asssets/config_file.png)
- main.cpp
  ![](/documentation/wifi_router_config/asssets/main_import.png)  
  ![](/documentation/wifi_router_config/asssets/main_usage.png)