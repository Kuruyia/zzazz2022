// Create the maps directory
Deno.mkdirSync('maps/', {recursive: true});

// Check each file
for (const dir_entry of Deno.readDirSync('maps/')) {
    // Only check .bin files
    if (!dir_entry.name.endsWith('.bin') || !dir_entry.isFile) {
        continue;
    }

    // Check the file size
    const path: string = 'maps/' + dir_entry.name;
    const stats: Deno.FileInfo = Deno.lstatSync(path);

    if (stats.size != 544) {
        continue;
    }

    // Check the file hash
    const file_data: Uint8Array = Deno.readFileSync(path);
    const digest: ArrayBuffer = await crypto.subtle.digest('SHA-1', file_data);
    const digest_view: Uint8Array = new Uint8Array(digest);
    let digest_str = '';

    for (const n of digest_view) {
        digest_str += n.toString(16).padStart(2, '0');
    }

    if (digest_str != '4db3310f314c5665b273601aec830c9de61fed21') {
        continue;
    }

    Deno.removeSync(path);
}
