package dttv.app.model;

import java.io.File;


public class Item extends File {
	
	private static final long serialVersionUID = 1L;

	public Item(File dir, String name) {
		super(dir, name);
		this.file = name;
		// TODO Auto-generated constructor stub
	}

	public String file;
	private int icon;

	public int getIcon() {
		return icon;
	}

	public void setIcon(int icon) {
		this.icon = icon;
	}

	@Override
	public String toString() {
		return file;
	}
}
