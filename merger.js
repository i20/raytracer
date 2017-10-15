const fs = require('fs');
const readline = require('readline');

console.log('STARTING MERGER');

// Files to ignore
const excluded = ['.DS_Store'];

// To produce a valid single file program, we need to pay attention
// to the order header files are concatenated (dependencies)
// Then we can add all .cpp in an arbitrary order

fs.readdir('./headers', (err, files) => {

    let headers = [];

    for (let file of files) {

        if (excludes.includes(file)) continue;

        let deps = parseHeaderDependencies(file);

        headers.push(file);
    }

    console.log(files);
});

function parseHeaderDependencies (file) {

    const reader = readline.createInterface({
        input: fs.createReadStream(file)
    });

    reader.on('line', (line) => {

        let regexp = /#include\s"\.\.\/headers\/(.+)\.hpp"/;

        for (let line of data.split('\n')) {

            let match = regexp.exec(line);

            if (!match) continue;


        }
    });
}
