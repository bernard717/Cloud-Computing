1. last ������ ��ȭ�� ����ϴ� Docker �����̳� �̹��� �����
(1) �ش� c ����
last.c (÷��)
(2) last�� �������Ϸ� ����� ��ɾ�
gcc --static -o last last.c
(3) �ش� dockerfile
Dockerfile-last (÷��)
(4) �̹��� �����
docker build -t ubuntu:last .
(5) �����̳� build
docker run -it -v /var/lib/docker/volumes/world/_data:/test ubuntu:last
(6) �ٲ� ���ڿ� Ȯ�� ���
cd /var/lib/docker/volumes/world/_data
vi changes.txt

2. �Է¹��� ���ڿ��� /data/.last ���Ͽ� {UNIX time in ms},{���ڿ�} ���·� �����ϴ� Docker �����̳� �̹��� �����
(1) �ش� c ����
hello.c (÷��)
(2) last�� �������Ϸ� ����� ��ɾ�
gcc --static -o hello hello.c
(3) �ش� dockerfile
Dockerfile-hello (÷��)
(4) �̹��� �����
docker build -t ubuntu:hello .
(5) �����̳� build
docker run -it -v /var/lib/docker/volumes/world/_data:/test ubuntu:hello
(6) ���ڿ� �Է� ���
build���� �� "Write"��� ȭ�鿡 �߸� ���ڿ� �Է��ϰ� Enter
(7) �Էµ� ���ڿ� Ȯ�� ���
cd /var/lib/docker/volumes/world/_data
vi .last 

