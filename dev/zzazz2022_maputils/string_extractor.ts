import { CHARCODES } from './utils.ts';

// Create the maps directory
Deno.mkdirSync('maps/', {recursive: true});

// Check each file for strings
for (const dir_entry of Deno.readDirSync('maps/')) {
    // Only check .bin files
    if (!dir_entry.name.endsWith('.bin') || !dir_entry.isFile) {
        continue;
    }

    // Read the file
    const path: string = 'maps/' + dir_entry.name;
    let file_data: Uint8Array = Deno.readFileSync(path);

    // Remove all unknown characters
    for (let i = 0; i < file_data.length; ++i) {
        if (CHARCODES && CHARCODES.has(file_data[i])) {
            const char_code: number | undefined = CHARCODES.get(file_data[i])?.charCodeAt(0);

            if (char_code) {
                file_data[i] = char_code;
            } else {
                file_data[i] = 0xFF;
            }
        } else {
            file_data[i] = 0xFF;
        }
    }

    // Save the ASCII version
    const ascii_path = 'maps/' + dir_entry.name.replace('.bin', '') + '.ascii.bin';
    Deno.writeFileSync(ascii_path, file_data);
}
