from PIL import Image
L_ratio = 4
W_ratio = 8

def getPixels(image_path):
    img = Image.open(image_path).convert('RGB')
    width, height = img.size
    new_width = width // W_ratio
    new_height = height // L_ratio
    print(f"Original size: {width}x{height}, New size: {new_width}x{new_height}")

    pixels = [[0x00 for _ in range(new_width)] for _ in range(new_height)]
    def get_color_byte(r, g, b):
        if r < 40 and g < 40 and b < 40:
            return 0x00

        # Greens
        if g > r and g > b:
            if g > 200 and r < 100 and b < 100:
                return 0x33
            if g > 120 and r < 100 and b < 100:
                return 0x22
            if g > 60 and r < 80 and b < 80:
                return 0x11

        # Blues
        if b > r and b > g:
            if b > 220 and g > 200:
                return 0x77
            if b > 220:
                return 0x66
            if b > 130:
                return 0x55
            if b > 60:
                return 0x44

        # Reds & Oranges
        if r > g and r > b:
            if r > 200 and g < 100 and b < 100:
                return 0x88
            if r > 200 and 140 < g < 200:
                return 0xAA
            if r > 180 and 80 < g <= 140:
                return 0x99

        # Yellows
        if r > 200 and g > 200 and b < 100:
            return 0xBB

        # Pinks & Magentas
        if r > 180 and b > 180:
            if g > 140:
                return 0xEE
            if g > 60:
                return 0xDD
            return 0xCC

        # White
        if r > 230 and g > 230 and b > 230:
            return 0xFF

        return 0x00 # Default fallback

    # def get_color_byte(r, g, b):
    #     if r < 50 and g < 50 and b < 50:
    #         return 0x00  # Black
    #     elif r > 220 and g > 220 and b > 220:
    #         return 0x00  # White
    #     elif r > g and r > b:
    #         return 0x88  # Red
    #     else:
    #         return 0x66  # Default is Blue

    for y in range(new_height):
        for x in range(new_width):
            if x == 0 or y == 0 or x == new_width - 1 or y == new_height - 1:
                pixels[y][x] = 0xFF
            else:
                r, g, b = img.getpixel((x * W_ratio, y * L_ratio))
                pixels[y][x] = get_color_byte(r, g, b)

    # Array Format
    for row in pixels[2:-3]:
        print("{" + ", ".join(f"0x{val:02X}" for val in row[1:-1]) + "},")

image_path = "./content/24-big.png"
getPixels(image_path)