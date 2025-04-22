function decodeUplink(input) {
  const rawString = String.fromCharCode.apply(null, input.bytes);
  const entries = rawString.split("#").filter(e => e.trim().length > 0);

  const data = entries.map(entry => {
    try {
      const [pi_id, num_faces, timestampCompact] = entry.split(";");

      const year = parseInt(timestampCompact.slice(0, 4));
      const month = parseInt(timestampCompact.slice(4, 6)) - 1; // JavaScript months = 0-11
      const day = parseInt(timestampCompact.slice(6, 8));
      const hour = parseInt(timestampCompact.slice(8, 10));
      const minute = parseInt(timestampCompact.slice(10, 12));
      const second = parseInt(timestampCompact.slice(12, 14));
      const millisecond = parseInt(timestampCompact.slice(14, 17));

      const timestamp = new Date(year, month, day, hour, minute, second, millisecond).toISOString();

      return {
        pi_id,
        num_faces: parseInt(num_faces),
        timestamp
      };
    } catch (err) {
      return { error: "Failed to parse entry", raw: entry };
    }
  });

  return {
    data: {
      decoded_faces: data
    },
    warnings: [],
    errors: []
  };
}
