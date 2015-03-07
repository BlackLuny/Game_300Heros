/**
 * Created by 201724 on 2015/3/7.
 */

var fs = require('fs');
var path = require('path');
var zlib = require('zlib');

function FSNode_Create(name){
	var pub = {
		name : name,
		position : 0,
		origin_size : 0,
		compr_size : 0,
	};

	return pub;
}

function FS_Create(fd){

	var pri = {
		fd : fd,
		position : 0,

		setPos : function (pos){
			this.position = pos;
		},

		readString: function(length){
			var buffer = this.readBytes(length);
			for(var i = 0; i < buffer.length ; i++){
				if(buffer[i] == 0){
					var new_buf = new Buffer(i);
					buffer.copy(new_buf,0,0,i);
					buffer = null;
					var s = new_buf.toString();
					new_buf = null;
					return s;
				}
			}
			return '';
		},

		readBytes : function (count) {
			try
			{
				var buffer = new Buffer(count);

				fs.readSync(fd, buffer, 0, count, this.position);

				this.position += count;
			}
			catch(e)
			{
				
			}
			return buffer;
		},

		readUInt32 : function (){
			var buffer = this.readBytes(0x4);

			var uint = buffer.readUIntLE(0,0x4);

			buffer = null;
			return uint;
		}
	};


	var pub = {
		header : {
			magic : null,
			count : 0
		},

		files : new Array(),

		getData: function (fs_node){
			if(fs_node.compr_size == 0){
				return new Buffer(0);
			}
			pri.setPos(fs_node.position);

			return pri.readBytes(fs_node.compr_size);
		}
	};

	
	pub.header.magic = pri.readString(0x32);
	console.log('magic => ' + pub.header.magic);

	pub.header.count = pri.readUInt32();
	console.log('f_count =>' + pub.header.count);

	for(var i = 0; i< pub.header.count ;i++){
		var file = FSNode_Create(pri.readString(0x104));

		file.position = pri.readUInt32();
		file.origin_size = pri.readUInt32();
		file.compr_size = pri.readUInt32();
		pri.readBytes(0x20);		//ignore

		pub.files.push(file);
	}

	return pub;
}

main();


function mkdirsSync(dirpath, mode) { 
    if (!fs.existsSync(dirpath)) {
        var pathtmp;
        dirpath.split(path.sep).forEach(function(dirname) {
            if (pathtmp) {
                pathtmp = path.join(pathtmp, dirname);
            }
            else {
                pathtmp = dirname;
            }
            if (!fs.existsSync(pathtmp)) {
                if (!fs.mkdirSync(pathtmp, mode)) {
                    return false;
                }
            }
        });
    }
    return true; 
}

function main()
{
    if(fs.existsSync('data.jmp') == false){
    	console.log('data.jmp don\'t exists!');
    	return;
    }

    var fd = fs.openSync('data.jmp', 'r');
    if(fd == null){
    	console.log('open file failed!');
    	return;
    }

    var package = FS_Create(fd);

    for(var index in package.files)
    {
    	var file = package.files[index];

    	console.log(file.name);

    	var f_path = 'output\\' + file.name.replace('..\\','');

    	mkdirsSync(path.dirname(f_path));

    	var compr_buf = package.getData(file);	
    	var source_buf = zlib.inflateSync(compr_buf);

    	var nfd = fs.openSync(f_path, 'w+');
    	if(nfd > 0){
    		fs.writeSync(nfd,source_buf,0,source_buf.length);
    		fs.closeSync(nfd);
    	}

    	compr_buf = null;
    	source_buf = null;

    }

    fs.closeSync(fd);
}

