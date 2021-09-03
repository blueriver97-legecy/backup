# Raspberry Pi 설정 파일

라즈베리파이 ssh, wifi, vimrc, network/interfaces 설정 파일.
---------------------------------------
Setup Guide
---------------------------------------

#### ssh
SD카드 내에 복사 후 라즈베리파이 부팅.
부팅 과정에서 SSH 파일 유무 검사 후 있으면 SSH 활성화.

#### vimrc
자주 사용하는 설정을 작성해놓은 vim 설정 파일.
```
$ apt-get install vim
$ cp interfaces /etc/vim/vimrc
```

#### wifi
SD카드 내에 복사 후 라즈베리파이 부팅.
부팅 과정에서 wpa_supplicant.conf 파일을 확인하여 작성된 내용으로 Wi-Fi 자동 연결(1회성)

접속 이후, /etc/network/ 경로에 interfaces 파일 작성한 뒤,
부팅시 Wi-Fi 자동 연결을 위해 /etc/wpa_supplicant 경로에 wpa_supplicant.conf 파일 작성

```
$ cp interfaces /etc/network/interfaces
$ cp wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf
```

#### interfaces

Static IP 사용 시, 해당 파일에서 주석 제거 후 사용.
