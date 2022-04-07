// Create the maps directory
Deno.mkdirSync('maps/', {recursive: true});

// Store the found uppercase chains
type ChainsType = {
    [map_id: string]: Array<string>;
}

let chains: ChainsType = {};

const isUppercase = (str: string): boolean => {
    return str == str.toUpperCase();
}

// Check each file for strings
for (const dir_entry of Deno.readDirSync('maps/')) {
    // Only check .ascii.bin files
    if (!dir_entry.name.endsWith('.ascii.bin') || !dir_entry.isFile) {
        continue;
    }

    // Read the file
    const path: string = 'maps/' + dir_entry.name;
    const file_data: string = Deno.readTextFileSync(path);

    // Get the map ID
    const map_id: string = dir_entry.name.slice(0, 4);

    // Look for a long chain of uppercase characters
    let map_chains: Array<string> = new Array<string>();
    let uppercase_regex = /[A-ZÀ-ÿ][A-ZÀ-ÿ\s]{4,}[A-ZÀ-ÿ]/g;

    for (const matches of file_data.matchAll(uppercase_regex)) {
        for (const match of matches) {
            map_chains.push(match);
        }
    }

    chains[map_id] = map_chains;
}

// Save the chains to a file
Deno.writeTextFileSync('maps/chains.txt', JSON.stringify(chains, null, 4));
