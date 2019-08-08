#!/bin/zsh

echo '{' >> NPN_LUT4.json
for f in *.v; 
do
    SEEN=false
    ROLL=false
    FIRST=true
    echo '\t'\"${f%??}\"":" \{ >> NPN_LUT4.json
    echo '\t\t'\"inputs\": \[\"a\", \"b\", \"c\", \"d\"\]"," >> NPN_LUT4.json
    echo '\t\t'\"outputs\": \"F\""," >> NPN_LUT4.json
    tail -n +7 $f | while read line
    do
        if [[ "$ROLL" = true ]]
        then
            echo ${line[0, -1]}\""," >> NPN_LUT4.json
            ROLL=false
            continue
        fi

        if [[ "$line" =~ "[[:space:]]*wire" ]]
        then
            if [[ "${line[-1]}" = ";" ]]
            then
                echo '\t\t'\"wires\": \"${line[6, -2]}\", >> NPN_LUT4.json
                continue
            fi
            ROLL=true
            echo -n '\t\t'\"wires\": \"${line[6, -2]}", " >> NPN_LUT4.json
            continue
        fi
        if [[ ! ("$line" =~ "endmodule") ]]
        then
            if [[ "$SEEN" = false ]]
            then
                        echo '\t\t'\"gates\": \[>> NPN_LUT4.json
                        SEEN=true
            fi
            if [[ "$FIRST" = true ]]
            then  
                echo -n '\t\t\t'\"$line\" >> NPN_LUT4.json
                FIRST=false
                continue
            fi
            echo -n ',\n\t\t\t'\"$line\" >> NPN_LUT4.json
        fi
    done
    echo '\n\t\t'\] >> NPN_LUT4.json
    echo '\t},' >> NPN_LUT4.json
done
echo '}' >> NPN_LUT4.json