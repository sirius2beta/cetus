# 複製 socat-forward.service
sudo cp socat-forward.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable socat-forward
sudo systemctl start socat-forward

# 設置symlink
sudo cp 99-cetus.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
