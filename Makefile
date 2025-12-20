all:
	rm -rf build && mkdir -p build
	clang -fPIC -shared panel-plugin/icon.c panel-plugin/processmanager.c panel-plugin/gtkgui.c -o build/libxfce4panel-icon.so \
	$$(pkg-config --cflags --libs gtk+-3.0 libxfce4panel-2.0)

install:
	sudo mkdir -p /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins
	sudo mkdir -p /usr/share/xfce4/panel/plugins
	sudo cp build/libxfce4panel-icon.so /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins/
	sudo cp panel-plugin/icon.desktop /usr/share/xfce4/panel/plugins/xfce4panel-icon.desktop
	xfce4-panel -q
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
	xfce4-panel -q
	xfce4-panel &
