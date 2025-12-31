all:
	rm -rf build && mkdir -p build
	clang -fPIC -shared panel-plugin/icon.c panel-plugin/processmanager.c panel-plugin/gtkgui.c -o build/libxfce4panel-icon.so \
	$$(pkg-config --cflags --libs gtk+-3.0 libxfce4panel-2.0)

run-gui:
	rm -rf build && mkdir -p build
	clang -DSTANDALONE_GUI panel-plugin/gtkgui.c panel-plugin/processmanager.c -o build/test-gui \
	$$(pkg-config --cflags --libs gtk+-3.0 libxfce4panel-2.0) && ./build/test-gui

install:
	sudo mkdir -p /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins
	sudo mkdir -p /usr/share/xfce4/panel/plugins
	sudo mkdir -p /etc/sudoers.d
	sudo mkdir -p /usr/local/bin/panel-plugin/helpers
	sudo cp build/libxfce4panel-icon.so /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins/
	sudo cp panel-plugin/icon.desktop /usr/share/xfce4/panel/plugins/xfce4panel-icon.desktop
	sudo install -m 755 panel-plugin/helpers/ss-network-viewer.sh /usr/local/bin/panel-plugin/helpers/ss-network-viewer.sh
	sudo install -m 755 panel-plugin/helpers/port-trace-kill.sh /usr/local/bin/panel-plugin/helpers/port-trace-kill.sh
	sudo cp panel-plugin/sudoers/port-trace.sudoers /etc/sudoers.d/port-trace
	sudo chmod 0440 /etc/sudoers.d/port-trace
	sudo chown root:root /etc/sudoers.d/port-trace
	xfce4-panel -q
	sleep 2
	xfce4-panel &

clean:
	rm -f build/libxfce4panel-icon.so
	rm -rf build

check:
	xfce4-panel -q
	xfce4-panel &

uninstall:
	sudo rm -f /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins/libxfce4panel-icon.so
	sudo rm -f /usr/share/xfce4/panel/plugins/xfce4panel-icon.desktop
	sudo rm -rf /usr/local/bin/panel-plugin/helpers
	sudo rm -f /etc/sudoers.d/port-trace
	xfce4-panel -q
	sleep 2
	xfce4-panel &
