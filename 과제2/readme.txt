1. last 파일의 변화를 기록하는 Docker 컨테이너 이미지 만들기
(1) 해당 c 파일
last.c (첨부)
(2) last를 실행파일로 만드는 명령어
gcc --static -o last last.c
(3) 해당 dockerfile
Dockerfile-last (첨부)
(4) 이미지 만들기
docker build -t ubuntu:last .
(5) 컨테이너 build
docker run -it -v /var/lib/docker/volumes/world/_data:/test ubuntu:last
(6) 바뀐 문자열 확인 방법
cd /var/lib/docker/volumes/world/_data
vi changes.txt

2. 입력받은 문자열을 /data/.last 파일에 {UNIX time in ms},{문자열} 형태로 저장하는 Docker 컨테이너 이미지 만들기
(1) 해당 c 파일
hello.c (첨부)
(2) last를 실행파일로 만드는 명령어
gcc --static -o hello hello.c
(3) 해당 dockerfile
Dockerfile-hello (첨부)
(4) 이미지 만들기
docker build -t ubuntu:hello .
(5) 컨테이너 build
docker run -it -v /var/lib/docker/volumes/world/_data:/test ubuntu:hello
(6) 문자열 입력 방법
build했을 때 "Write"라고 화면에 뜨면 문자열 입력하고 Enter
(7) 입력된 문자열 확인 방법
cd /var/lib/docker/volumes/world/_data
vi .last 

