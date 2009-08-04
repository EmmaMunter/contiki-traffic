/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: CoffeeFS.java,v 1.3 2009/08/04 15:19:08 nvt-se Exp $
 *
 * @author Nicolas Tsiftes
 *
 */

package se.sics.coffee;

import java.io.*;
import java.util.Map;
import java.util.TreeMap;

class CoffeeException extends RuntimeException {
	public CoffeeException(String message) {
		super("Coffee error: " + message);
	}
}

class CoffeeFileException extends RuntimeException {
	public CoffeeFileException(String message) {
		super("Coffee file error: " + message);
	}
}

public class CoffeeFS {
	private CoffeeImage image;
	private CoffeeConfiguration conf;
	private int currentPage;
	private Map<String, CoffeeFile> files;
	private static final int INVALID_PAGE = -1;

	public CoffeeFS(CoffeeImage image) throws IOException {
		this.image = image;
		conf = image.getConfiguration();
		currentPage = 0;
		files = new TreeMap<String, CoffeeFile>();

		while(currentPage < (conf.fsSize / conf.pageSize)) {
			CoffeeHeader header = readHeader(currentPage);
			if(header.isActive()) {
				CoffeeFile file = new CoffeeFile(this, header);
				files.put(file.getName(), file);
			}
			currentPage = nextFile(header, currentPage);
		}
	}

	private int pageCount(long size) {
		int headerSize = conf.NAME_LENGTH + conf.pageTypeSize * 2 + 6;
		return (int)(size + headerSize + conf.pageSize - 1) / conf.pageSize;
	}

	private int findFreeExtent(int pages) throws IOException {
		CoffeeHeader header;
		int currentPage = 0;
		int start = INVALID_PAGE;

		while(currentPage < (conf.fsSize / conf.pageSize)) {
			header = readHeader(currentPage);
			if(header.isFree()) {
				if(start == INVALID_PAGE) {
					start = currentPage;
				}
				currentPage = nextFile(header, currentPage);
				if(start + pages <= currentPage) {
					return start;
				}
			} else {
				start = INVALID_PAGE;
				currentPage = nextFile(header, currentPage);
			}
		}
		return INVALID_PAGE;
	}

	public CoffeeImage getImage() {
		return image;
	}

	public CoffeeConfiguration getConfiguration() {
		return conf;
	}

	private CoffeeHeader readHeader(int page) throws IOException {
		byte[] bytes = new byte[conf.NAME_LENGTH + conf.pageTypeSize * 2 + 6];
		int index = 0;

		image.read(bytes, bytes.length, page * conf.pageSize);
		CoffeeHeader header = new CoffeeHeader(this, page, bytes);

		return header;
	}

	private void writeHeader(CoffeeHeader header) throws IOException {
		byte[] bytes = header.toRawHeader();

		image.write(bytes, bytes.length, header.getPage() * conf.pageSize);
	}

	private int nextFile(CoffeeHeader header, int page) {
		if(header.isFree()) {
			return page + conf.pagesPerSector & ~(conf.pagesPerSector - 1);
		} else if(header.isIsolated()) {
			return page + 1;
		}

		return page + header.maxPages;
	}

	public final Map<String, CoffeeFile> getFiles() {
		return files;
	}

	public CoffeeFile insertFile(String filename) throws IOException {
		CoffeeFile coffeeFile;
		try {
			File file = new File(filename);
			FileInputStream input = new FileInputStream(file);
			int allocatePages = pageCount(file.length());
			int start = findFreeExtent(allocatePages);

			if(start == INVALID_PAGE) {
				return null;
			}
			CoffeeHeader header = new CoffeeHeader(this, start);
			header.setName(filename);
			header.setReservedSize(allocatePages);
			header.allocate();
			coffeeFile = new CoffeeFile(this, header);
			writeHeader(header);
			coffeeFile.insertContents(input);
			input.close();
			return coffeeFile;
		} catch (FileNotFoundException e) {
		}
		return null;
	}

	public void removeFile(String filename)
			throws CoffeeFileException, IOException {
		CoffeeFile file = files.get(filename);
		if(file == null) {
			throw new CoffeeFileException("Coffee: attempt to remove inexistent file");
		}

		file.remove();
		writeHeader(file.getHeader());
		files.remove(file.getName());
	}

	public boolean extractFile(String filename) throws IOException {
		CoffeeFile file = files.get(filename);

		if(file == null) {
			return false;
		}

		file.saveContents(filename);
		return true;
	}
}
