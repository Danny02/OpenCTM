var exec = require('child_process').exec,
	libPath = require('path');

var ctmConvPath = libPath.join(__dirname, '/../tools/');

var ctmConvCommand = libPath.join(ctmConvPath, "ctmconv");
var ctmVConvEnv = {LD_LIBRARY_PATH : ctmConvPath};

exports.execute = function (parameters, callback) {
	var vtkSurfacePath = parameters.HackActionsHandler.getAction('mesh2vtk').attributes.path;

	var filesRoot = parameters.filesRoot;
	var inputMesh = libPath.join(filesRoot, parameters.input_mesh);
	var outputDirectory = libPath.join(filesRoot, parameters.output_directory);

	var execOptions = {cwd :outputDirectory, env: ctmVConvEnv};
	var plyMesh = "mesh.ply";

	switch (libPath.extname(inputMesh)) {
	case ".vtk":
		exec(libPath.join(vtkSurfacePath, "vtk2ply") + " " + inputMesh,
			execOptions, ctmConv);
		break;
	case ".stl":
		exec(libPath.join(vtkSurfacePath, "stl2ply") + " " + inputMesh,
			execOptions, ctmConv);
		break;
	default:
		plyMesh = inputMesh;
		ctmConv(null);
	}

	function ctmConv(err, stdout, stderr) {
		if (err) {
			callback(err, stdout, stderr);
		}
		exec(ctmConvCommand + " " + plyMesh + " mesh.ctm", execOptions, function (err, stdout, stderr) {
				callback(err, stdout, stderr);
		});
	}
}
