import os
import chardet

folders=["SipClient","SipService"]
for folder in folders:
    for root, dirs, files in os.walk(folder):
        for filename in files:
            filepath = os.path.join(root, filename)
            extension = os.path.splitext(filepath)[-1]
            
            if extension == ".h" or extension == ".cpp":
                with open(filepath, 'rb') as file:
                    rawdata = file.read()
                    result = chardet.detect(rawdata)
                
                    encoding = result['encoding']
                    confidence = result['confidence']
                    
                    if encoding != "UTF-8-SIG":
                        print(filepath+" ------------> "+extension)
                        print(encoding)
                        print(confidence)
    