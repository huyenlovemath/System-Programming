
Backdoor(client): host = localhost, port = 4444
Server: ncat -l -p 4444

- backdoor.exe thực thi lần đầu sẽ 
	+ Tự động cài đặt tới %SystemDrive%\Users\%USERNAME%\.Chrome\ChromeUpdate\backdoor.exe
	+ Thêm vào Computer\HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run để khởi động cùng hệ điều hành
        + Tạo mới process để thực thi file backdoor.exe mới với tham số truyền vào là link dẫn đến file cũ => xóa file cũ

- Ở phía server, nhập câu lệnh, backdoor nhận lệnh và thực hiện.
