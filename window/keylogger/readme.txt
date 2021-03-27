KEYLOGGER MALWARE:
 
- keylogger.exe thực thi lần đầu sẽ:
	+ Tự động cài đặt tới %SystemDrive%\Users\%USERNAME%\.Chrome\ChromeUpdate\keylogger.exe (trong hàm install())
	+ Thêm vào Computer\HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run để khởi động cùng hệ điều hành (trong hàm install())
	+ Tạo mới process thực thi file keylogger.exe với tham số truyền vào là link dẫn đến file cũ => xóa file cũ
- Thực thi ghi hoạt động của bàn phím (chưa xét tổ hợp phím) ra file keyboard.log (cùng thư mục với file keylogger.exe cài đặt ban đầu) 
 	+ Định dạng:
  		[Thời gian][Cửa sổ] <<chuỗi ký tự>>
  	+ Thời gian + tên cửa sổ được thêm sau mỗi lần thay đổi cửa sổ làm việc.
 	+ Nếu file keyboard.log đã tồn tại, thực hiện viết tiếp vào cuối file.
