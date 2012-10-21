var exec = require('child_process').exec,
	libPath=require('path'),
	settings=require('./config.json');

var vtkSurfacePath=settings.vtkSurfacePath;
var ctmConvPath=settings.ctmConvPath;

var ctmConvCommand=ctmConvPath+"ctmconv";
var ctmVConvEnv={LD_LIBRARY_PATH : ctmConvPath};

exports.execute=function (parameters, callback) {
	var filesRoot=parameters.filesRoot;
	var inputMesh=filesRoot+parameters.input_mesh;
	var outputDirectory=filesRoot+parameters.output_directory;

	var execOptions={cwd :outputDirectory, env: ctmVConvEnv};
	var plyMesh="mesh.ply";

	switch (libPath.extname(inputMesh))
	{
	case ".vtk":
		exec(vtkSurfacePath+"vtk2ply "+inputMesh, execOptions, ctmConv);
		break;
	case ".stl":
		exec(vtkSurfacePath+"stl2ply "+inputMesh, execOptions, ctmConv);
		break;
	default:
		plyMesh=inputMesh;
		ctmConv(null);
	}

	function ctmConv(err, stdout, stderr) {
		if (err) {
			callback(err, stdout, stderr);
		}
		exec(ctmConvCommand+" "+plyMesh+" mesh.ctm", execOptions, function (err, stdout, stderr) {
				callback(err, stdout, stderr);
		});
	}
}
